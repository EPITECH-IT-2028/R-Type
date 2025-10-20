#include "Server.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>
#include "Broadcast.hpp"
#include "Events.hpp"
#include "GameManager.hpp"
#include "IPacket.hpp"
#include "Macro.hpp"
#include "Packet.hpp"

/**
 * @brief Constructs a Server configured with the listening port and client
 * limits.
 *
 * Initializes internal counters, creates the GameManager with the specified
 * per-room client limit, and resizes internal client storage to accommodate
 * the maximum number of clients.
 *
 * @param port UDP port the server will bind to for network communication.
 * @param max_clients Maximum total concurrent clients the server will accept.
 * @param max_clients_per_room Maximum number of clients allowed in a single
 * game room.
 */
server::Server::Server(std::uint16_t port, std::uint8_t max_clients,
                       std::uint8_t max_clients_per_room)
    : _networkManager(port),
      _max_clients(max_clients),
      _max_clients_per_room(max_clients_per_room),
      _port(port),
      _player_count(0),
      _projectile_count(0),
      _next_player_id(0) {
  _gameManager = std::make_shared<game::GameManager>(_max_clients_per_room);
  _clients.resize(_max_clients);
}

void server::Server::start() {
  std::cout << "[CONSOLE] Server started on port " << _port << std::endl;

  _networkManager.setStopCallback([this]() {
    static bool stopping = false;
    if (stopping)
      return;
    stopping = true;
    std::cout << "[CONSOLE] Network manager stopped, shutting down server..."
              << std::endl;
    _gameManager->shutdownRooms();
  });

  startReceive();

  _networkManager.scheduleEventProcessing(std::chrono::milliseconds(50),
                                          [this]() { processGameEvents(); });
  _networkManager.scheduleTimeout(std::chrono::seconds(1),
                                  [this]() { handleTimeout(); });

  _networkManager.run();
}

void server::Server::processGameEvents() {
  auto rooms = _gameManager->getAllRooms();

  for (auto &room : rooms) {
    if (!room || !room->isActive()) {
      continue;
    }

    queue::GameEvent event;

    while (room->getGame().getEventQueue().popRequest(event)) {
      handleGameEvent(event, room->getRoomId());
    }
  }

  _gameManager->removeEmptyRooms();
}

void server::Server::stop() {
  std::cout << "[CONSOLE] Server stopped..." << std::endl;
  _networkManager.closeSocket();
  _gameManager->shutdownRooms();
}

void server::Server::handleTimeout() {
  auto now = std::chrono::steady_clock::now();

  if (_clients.empty()) {
    return;
  }

  for (std::size_t i = 0; i < _clients.size(); ++i) {
    auto &client = _clients[i];
    if (!client || !client->_connected)
      continue;
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(
        now - client->_last_heartbeat);
    if (duration.count() > CLIENT_TIMEOUT) {
      const int pid = client->_player_id;
      const int roomId = client->_room_id;
      std::cout << "[WORLD] Player " << pid << " timed out due to inactivity."
                << std::endl;
      client->_connected = false;
      if (_player_count > 0)
        --_player_count;

      if (roomId != NO_ROOM) {
        auto room = _gameManager->getRoom(roomId);
        if (room) {
          room->getGame().destroyPlayer(pid);
          room->removeClient(pid);

          auto roomClients = room->getClients();

          auto disconnectPacket = PacketBuilder::makePlayerDisconnect(pid);
          broadcast::Broadcast::broadcastPlayerDisconnectToRoom(
              _networkManager, roomClients, disconnectPacket);
        }
      }

      client.reset();
    }
  }
}

/**
 * @brief Convert a game event into its network packet and broadcast it to a
 * room.
 *
 * Converts the provided `queue::GameEvent` into the corresponding network
 * packet and broadcasts that packet to every client currently in the specified
 * room. If `roomId` equals `NO_ROOM` or the room is not found/active, the
 * function does nothing.
 *
 * @param event Variant holding the specific game event to translate and send.
 * @param roomId Identifier of the target room whose clients will receive the
 * packet.
 */
void server::Server::handleGameEvent(const queue::GameEvent &event,
                                     std::uint32_t roomId) {
  if (roomId == NO_ROOM) {
    return;
  }
  auto room = _gameManager->getRoom(roomId);
  if (!room) {
    return;
  }

  auto clients = room->getClients();

  std::visit(
      [this, &clients](const auto &specificEvent) {
        using T = std::decay_t<decltype(specificEvent)>;

        if constexpr (std::is_same_v<T, queue::EnemySpawnEvent>) {
          auto enemySpawnPacket = PacketBuilder::makeEnemySpawn(
              specificEvent.enemy_id, EnemyType::BASIC_FIGHTER, specificEvent.x,
              specificEvent.y, specificEvent.vx, specificEvent.vy,
              specificEvent.health, specificEvent.max_health);
          broadcast::Broadcast::broadcastEnemySpawnToRoom(
              _networkManager, clients, enemySpawnPacket);
        } else if constexpr (std::is_same_v<T, queue::EnemyDestroyEvent>) {
          auto enemyDeathPacket = PacketBuilder::makeEnemyDeath(
              specificEvent.enemy_id, specificEvent.x, specificEvent.y,
              specificEvent.player_id, specificEvent.score);
          broadcast::Broadcast::broadcastEnemyDeathToRoom(
              _networkManager, clients, enemyDeathPacket);
        } else if constexpr (std::is_same_v<T, queue::EnemyHitEvent>) {
          auto enemyHitPacket = PacketBuilder::makeEnemyHit(
              specificEvent.enemy_id, specificEvent.x, specificEvent.y,
              specificEvent.damage, specificEvent.sequence_number);
          broadcast::Broadcast::broadcastEnemyHitToRoom(
              _networkManager, clients, enemyHitPacket);
        } else if constexpr (std::is_same_v<T, queue::EnemyMoveEvent>) {
          auto enemyMovePacket = PacketBuilder::makeEnemyMove(
              specificEvent.enemy_id, specificEvent.x, specificEvent.y,
              specificEvent.vx, specificEvent.vy,
              specificEvent.sequence_number);
          broadcast::Broadcast::broadcastEnemyMoveToRoom(
              _networkManager, clients, enemyMovePacket);
        } else if constexpr (std::is_same_v<T, queue::ProjectileSpawnEvent>) {
          auto projectileSpawnPacket = PacketBuilder::makeProjectileSpawn(
              specificEvent.projectile_id, specificEvent.type, specificEvent.x,
              specificEvent.y, specificEvent.vx, specificEvent.vy,
              specificEvent.is_enemy_projectile, specificEvent.damage,
              specificEvent.owner_id);
          broadcast::Broadcast::broadcastProjectileSpawnToRoom(
              _networkManager, clients, projectileSpawnPacket);
        } else if constexpr (std::is_same_v<T, queue::PlayerHitEvent>) {
          auto playerHitPacket = PacketBuilder::makePlayerHit(
              specificEvent.player_id, specificEvent.damage, specificEvent.x,
              specificEvent.y, specificEvent.sequence_number);
          broadcast::Broadcast::broadcastPlayerHitToRoom(
              _networkManager, clients, playerHitPacket);
        } else if constexpr (std::is_same_v<T, queue::ProjectileDestroyEvent>) {
          auto projectileDestroyPacket = PacketBuilder::makeProjectileDestroy(
              specificEvent.projectile_id, specificEvent.x, specificEvent.y);
          broadcast::Broadcast::broadcastProjectileDestroyToRoom(
              _networkManager, clients, projectileDestroyPacket);
        } else if constexpr (std::is_same_v<T, queue::PlayerDestroyEvent>) {
          auto playerDestroyPacket = PacketBuilder::makePlayerDeath(
              specificEvent.player_id, specificEvent.x, specificEvent.y);
          broadcast::Broadcast::broadcastPlayerDeathToRoom(
              _networkManager, clients, playerDestroyPacket);
        } else if constexpr (std::is_same_v<T, queue::GameStartEvent>) {
          auto gameStartPacket =
              PacketBuilder::makeGameStart(specificEvent.game_started);
          broadcast::Broadcast::broadcastGameStartToRoom(
              _networkManager, clients, gameStartPacket);
        } else if constexpr (std::is_same_v<T, queue::PositionEvent>) {
          auto positionPacket = PacketBuilder::makePlayerMove(
              specificEvent.player_id, specificEvent.x, specificEvent.y,
              specificEvent.sequence_number);
          broadcast::Broadcast::broadcastPlayerMoveToRoom(
              _networkManager, clients, positionPacket);
        } else {
          std::cerr << "[WARNING] Unhandled game event type." << std::endl;
        }
      },
      event);
}

/*
 * Begin asynchronous receive operation.
 */
void server::Server::startReceive() {
  _networkManager.startReceive([this](const char *data, std::size_t size) {
    handleReceive(data, size);
  });
}

/**
 * @brief Process a received network packet and dispatch it to the appropriate
 * handler.
 *
 * Parses the packet header; if the packet is a PlayerInfo packet it handles
 * player connection setup, otherwise it resolves the sender to an existing
 * client and forwards the packet for client-specific processing. If header
 * deserialization fails or the sender cannot be resolved to a connected client,
 * the packet is ignored.
 *
 * @param data Pointer to the received data buffer.
 * @param bytes_transferred Number of bytes available in the buffer.
 */
void server::Server::handleReceive(const char *data,
                                   std::size_t bytes_transferred) {
  serialization::Buffer buffer(data, data + bytes_transferred);

  auto headerOpt =
      serialization::BitserySerializer::deserialize<PacketHeader>(buffer);

  if (!headerOpt) {
    std::cerr << "[WARNING] Failed to deserialize packet header" << std::endl;
    return;
  }

  PacketHeader header = headerOpt.value();

  if (header.type == PacketType::PlayerInfo) {
    handlePlayerInfoPacket(data, bytes_transferred);
    return;
  }

  int client_idx = findExistingClient();
  if (client_idx == KO)
    return;
  handleClientData(client_idx, data, bytes_transferred);
}

/**
 * @brief Handle a PlayerInfo packet from a client attempting to connect.
 *
 * Validates the packet size, checks if the client is already connected,
 * and if not, assigns a new player ID and registers the client.
 * If the server is at max capacity, it refuses the connection.
 * @param data Pointer to the received data buffer.
 * @param size Size of the received data buffer.
 */
void server::Server::handlePlayerInfoPacket(const char *data,
                                            std::size_t size) {
  auto current_endpoint = _networkManager.getRemoteEndpoint();

  for (size_t i = 0; i < _clients.size(); ++i) {
    if (_clients[i] && _clients[i]->_connected &&
        _networkManager.getClientEndpoint(_clients[i]->_player_id) ==
            current_endpoint) {
      return;
    }
  }

  for (size_t i = 0; i < _clients.size(); ++i) {
    if (!_clients[i]) {
      int id = _next_player_id++;
      _clients[i] = std::make_shared<Client>(id);
      _clients[i]->_connected = true;
      _player_count++;
      _networkManager.registerClient(id, current_endpoint);

      std::cout << "[WORLD] New player connecting with ID " << id << std::endl;

      auto handler = _factory.createHandler(PacketType::PlayerInfo);
      if (handler) {
        handler->handlePacket(*this, *_clients[i], data, size);
      }
      return;
    }
  }

  std::cerr << "[WARNING] Max clients reached. Refused connection from "
            << current_endpoint.address().to_string() << std::endl;
}

/**
 * @brief Find an existing client based on the sender's endpoint.
 * Compares the remote endpoint of the incoming packet with the stored
 * endpoints of connected clients. If a match is found, updates the client's
 * last heartbeat timestamp and returns the client's index. If no match is
 * found, returns KO.
 * @return int Index of the existing client, or KO if not found.
 */
size_t server::Server::findExistingClient() {
  auto current_endpoint = _networkManager.getRemoteEndpoint();

  for (size_t i = 0; i < _clients.size(); ++i) {
    if (_clients[i] && _clients[i]->_connected &&
        _networkManager.getClientEndpoint(_clients[i]->_player_id) ==
            current_endpoint) {
      _clients[i]->_last_heartbeat = std::chrono::steady_clock::now();
      return static_cast<int>(i);
    }
  }
  return KO;
}

/*
 * Process data received from a client.
 */
void server::Server::handleClientData(std::size_t client_idx, const char *data,
                                      std::size_t size) {
  if (client_idx < 0 || client_idx >= static_cast<size_t>(_clients.size()) ||
      !_clients[client_idx]) {
    return;
  }

  auto client = _clients[client_idx];

  serialization::Buffer buffer(data, data + size);

  auto headerOpt =
      serialization::BitserySerializer::deserialize<PacketHeader>(buffer);
  if (!headerOpt) {
    std::cerr << "[WARNING] Failed to deserialize packet header from client "
              << _clients[client_idx]->_player_id << std::endl;
    return;
  }

  PacketHeader header = headerOpt.value();

  auto handler = _factory.createHandler(header.type);
  if (handler) {
    handler->handlePacket(*this, *client, data, size);
  } else {
    std::cerr << "[WARNING] Unknown packet type "
              << static_cast<int>(header.type) << " from client "
              << _clients[client_idx]->_player_id << std::endl;
  }
}

bool server::Server::initializePlayerInRoom(Client &client) {
  if (client._state == ClientState::CONNECTED_MENU) {
    std::cerr << "[ERROR] Cannot initialize player " << client._player_id
              << " - still in menu" << std::endl;
    return false;
  }

  if (client._room_id == NO_ROOM) {
    std::cerr << "[ERROR] Cannot initialize player " << client._player_id
              << " not in any room" << std::endl;
    return false;
  }

  if (client._player_name.empty()) {
    std::cerr << "[ERROR] Cannot initialize player " << client._player_id
              << " no name set" << std::endl;
    return false;
  }

  auto room = _gameManager->getRoom(client._room_id);
  if (!room) {
    std::cerr << "[ERROR] Cannot initialize player " << client._player_id
              << " room " << client._room_id << " not found" << std::endl;
    return false;
  }

  auto player =
      room->getGame().createPlayer(client._player_id, client._player_name);
  if (!player) {
    std::cerr << "[ERROR] Cannot initialize player " << client._player_id
              << " failed to create player entity in room " << client._room_id
              << std::endl;
    return false;
  }

  client._entity_id = player->getEntityId();

  std::pair<float, float> pos = player->getPosition();
  float speed = player->getSpeed();
  int max_health = player->getMaxHealth().value_or(100);

  auto ownPlayerPacket = PacketBuilder::makeNewPlayer(
      client._player_id, pos.first, pos.second, speed, max_health);
  auto serializedBuffer =
      serialization::BitserySerializer::serialize(ownPlayerPacket);
  _networkManager.sendToClient(
      client._player_id,
      reinterpret_cast<const char *>(serializedBuffer.data()),
      serializedBuffer.size());

  auto roomClients = room->getClients();

  broadcast::Broadcast::broadcastExistingPlayersToRoom(
      _networkManager, room->getGame(), client._player_id, roomClients);

  auto newPlayerPacket = PacketBuilder::makeNewPlayer(
      client._player_id, pos.first, pos.second, speed, max_health);
  broadcast::Broadcast::broadcastAncientPlayerToRoom(
      _networkManager, roomClients, newPlayerPacket);

  if (roomClients.size() >= 2 &&
      room->getState() == game::RoomStatus::WAITING) {
    room->startCountdown(COUNTDOWN_TIME);

    auto timer = std::make_shared<asio::steady_timer>(
        _networkManager.getIoContext(), std::chrono::seconds(1));

    handleCountdown(room, timer);
  }

  std::cout << "[WORLD] Player " << client._player_id << " ("
            << client._player_name << ") initialized in room "
            << client._room_id << std::endl;

  return true;
}

void server::Server::handleCountdown(
    std::shared_ptr<game::GameRoom> room,
    std::shared_ptr<asio::steady_timer> timer) {
  if (!room || room->getState() != game::RoomStatus::STARTING) {
    return;
  }

  int countdown = room->getCountdownValue();
  auto roomClients = room->getClients();

  if (countdown <= 0) {
    auto startPacket = PacketBuilder::makeGameStart(true);
    broadcast::Broadcast::broadcastGameStartToRoom(_networkManager, roomClients,
                                                   startPacket);
    room->start();
    for (auto &roomClient : roomClients) {
      if (roomClient) {
        roomClient->_state = ClientState::IN_GAME;
      }
    }

    std::cout << "[ROOM] Game started in room " << room->getRoomId()
              << std::endl;
    return;
  }

  std::cout << "[ROOM] Countdown " << countdown << " for room "
            << room->getRoomId() << std::endl;

  room->decrementCountdown();

  timer->expires_after(std::chrono::seconds(1));
  timer->async_wait([this, room, timer](const asio::error_code &error) {
    if (!error) {
      handleCountdown(room, timer);
    } else {
      std::cerr << "[ERROR] Timer error in countdown: " << error.message()
                << std::endl;
    }
  });
}

/*
 * Retrieve a client by index.
 * Returns nullptr if the index is invalid or the client does not exist.
 */
std::shared_ptr<server::Client> server::Server::getClient(
    std::size_t idx) const {
  if (idx < 0 || idx >= static_cast<std::size_t>(_clients.size())) {
    return nullptr;
  }
  return _clients[idx];
}

std::shared_ptr<server::Client> server::Server::getClientById(
    int player_id) const {
  for (const auto &client : _clients) {
    if (client && client->_player_id == player_id) {
      return client;
    }
  }
  return nullptr;
}

void server::Server::clearClientSlot(int player_id) {
  for (auto &client : _clients) {
    if (client && client->_player_id == player_id) {
      if (client->_room_id != NO_ROOM)
        _gameManager->leaveRoom(client);
      client.reset();
      return;
    }
  }
}
