#include "Server.hpp"
#include <chrono>
#include <cstddef>
#include <cstring>
#include <iostream>
#include "Broadcast.hpp"
#include "Events.hpp"
#include "IPacket.hpp"
#include "Macro.hpp"
#include "Packet.hpp"
#include "PacketSender.hpp"

server::Client::Client(int id) : _player_id(id) {
  _connected = true;
  _last_heartbeat = std::chrono::steady_clock::now();
  _last_position_update = std::chrono::steady_clock::now();
}

server::Server::Server(std::uint16_t port, std::uint16_t max_clients)
    : _networkManager(port),
      _max_clients(max_clients),
      _port(port),
      _player_count(0),
      _projectile_count(0),
      _next_player_id(0) {
  _clients.resize(max_clients);
}

void server::Server::start() {
  std::cout << "[CONSOLE] Server started on port " << _port << std::endl;
  _game.start();

  _networkManager.setStopCallback([this]() {
    static bool stopping = false;
    if (stopping)
      return;
    stopping = true;
    std::cout << "[CONSOLE] Network manager stopped, shutting down server..."
              << std::endl;
    _game.stop();
  });

  startReceive();

  _networkManager.scheduleEventProcessing(std::chrono::milliseconds(50),
                                          [this]() { processGameEvents(); });
  _networkManager.scheduleTimeout(std::chrono::seconds(1),
                                  [this]() { handleTimeout(); });

  _networkManager.run();
}

void server::Server::processGameEvents() {
  queue::GameEvent event;

  while (_game.getEventQueue().popRequest(event)) {
    handleGameEvent(event);
  }
}

void server::Server::stop() {
  std::cout << "[CONSOLE] Server stopped..." << std::endl;
  _networkManager.closeSocket();
  _game.stop();
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
      std::cout << "[WORLD] Player " << pid << " timed out due to inactivity."
                << std::endl;
      client->_connected = false;
      if (_player_count > 0)
        --_player_count;

      if (auto player = _game.getPlayer(pid)) {
        _game.destroyPlayer(pid);
      }
      auto disconnectPacket = PacketBuilder::makePlayerDisconnect(pid);
      client.reset();
      broadcast::Broadcast::broadcastPlayerDisconnect(_networkManager, _clients,
                                                      disconnectPacket);
    }
  }
}

/**
 * @brief Translate a game event into its network packet and broadcast it to all
 * connected clients.
 *
 * Handles each concrete variant of `queue::GameEvent` (EnemySpawnEvent,
 * EnemyDestroyEvent, EnemyHitEvent, EnemyMoveEvent, ProjectileSpawnEvent,
 * ProjectileDestroyEvent, PlayerHitEvent, PlayerDestroyEvent) by building the
 * corresponding network packet and broadcasting it to every connected client
 * via the server's UDP socket.
 *
 * @param event Variant containing the specific game event to handle.
 */
void server::Server::handleGameEvent(const queue::GameEvent &event) {
  std::visit(
      [this](const auto &specificEvent) {
        using T = std::decay_t<decltype(specificEvent)>;

        if constexpr (std::is_same_v<T, queue::EnemySpawnEvent>) {
          auto enemySpawnPacket = PacketBuilder::makeEnemySpawn(
              specificEvent.enemy_id, EnemyType::BASIC_FIGHTER, specificEvent.x,
              specificEvent.y, specificEvent.vx, specificEvent.vy,
              specificEvent.health, specificEvent.max_health);
          broadcast::Broadcast::broadcastEnemySpawn(_networkManager, _clients,
                                                    enemySpawnPacket);
        } else if constexpr (std::is_same_v<T, queue::EnemyDestroyEvent>) {
          auto enemyDeathPacket = PacketBuilder::makeEnemyDeath(
              specificEvent.enemy_id, specificEvent.x, specificEvent.y,
              specificEvent.player_id, specificEvent.score);
          broadcast::Broadcast::broadcastEnemyDeath(_networkManager, _clients,
                                                    enemyDeathPacket);
        } else if constexpr (std::is_same_v<T, queue::EnemyHitEvent>) {
          auto enemyHitPacket = PacketBuilder::makeEnemyHit(
              specificEvent.enemy_id, specificEvent.x, specificEvent.y,
              specificEvent.damage, specificEvent.sequence_number);
          broadcast::Broadcast::broadcastEnemyHit(_networkManager, _clients,
                                                  enemyHitPacket);
        } else if constexpr (std::is_same_v<T, queue::EnemyMoveEvent>) {
          auto enemyMovePacket = PacketBuilder::makeEnemyMove(
              specificEvent.enemy_id, specificEvent.x, specificEvent.y,
              specificEvent.vx, specificEvent.vy,
              specificEvent.sequence_number);
          broadcast::Broadcast::broadcastEnemyMove(_networkManager, _clients,
                                                   enemyMovePacket);
        } else if constexpr (std::is_same_v<T, queue::ProjectileSpawnEvent>) {
          auto projectileSpawnPacket = PacketBuilder::makeProjectileSpawn(
              specificEvent.projectile_id, specificEvent.type, specificEvent.x,
              specificEvent.y, specificEvent.vx, specificEvent.vy,
              specificEvent.is_enemy_projectile, specificEvent.damage,
              specificEvent.owner_id);
          broadcast::Broadcast::broadcastProjectileSpawn(
              _networkManager, _clients, projectileSpawnPacket);
        } else if constexpr (std::is_same_v<T, queue::PlayerHitEvent>) {
          auto playerHitPacket = PacketBuilder::makePlayerHit(
              specificEvent.player_id, specificEvent.damage, specificEvent.x,
              specificEvent.y, specificEvent.sequence_number);
          broadcast::Broadcast::broadcastPlayerHit(_networkManager, _clients,
                                                   playerHitPacket);
        } else if constexpr (std::is_same_v<T, queue::ProjectileDestroyEvent>) {
          auto projectileDestroyPacket = PacketBuilder::makeProjectileDestroy(
              specificEvent.projectile_id, specificEvent.x, specificEvent.y);
          broadcast::Broadcast::broadcastProjectileDestroy(
              _networkManager, _clients, projectileDestroyPacket);
        } else if constexpr (std::is_same_v<T, queue::PlayerDestroyEvent>) {
          auto playerDestroyPacket = PacketBuilder::makePlayerDeath(
              specificEvent.player_id, specificEvent.x, specificEvent.y);
          broadcast::Broadcast::broadcastPlayerDeath(_networkManager, _clients,
                                                     playerDestroyPacket);
        } else if constexpr (std::is_same_v<T, queue::GameStartEvent>) {
          auto gameStartPacket =
              PacketBuilder::makeGameStart(specificEvent.game_started);
          broadcast::Broadcast::broadcastGameStart(_networkManager, _clients,
                                                   gameStartPacket);
        } else if constexpr (std::is_same_v<T, queue::PositionEvent>) {
          auto positionPacket = PacketBuilder::makePositionPlayer(
              specificEvent.player_id, specificEvent.x, specificEvent.y);
          broadcast::Broadcast::broadcastPositionUpdate(
              _networkManager, _clients, positionPacket);
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
 * @brief Handle incoming data from clients.
 *
 * Parses the packet header to determine the packet type and delegates to the
 * appropriate handler function. If the packet is a PlayerInfo packet, it calls
 * `handlePlayerInfoPacket` to manage new player connections. For other packet
 * types, it attempts to find the existing client based on the sender's
 * endpoint and processes the data accordingly.
 *
 * @param data Pointer to the received data buffer.
 * @param bytes_transferred Number of bytes received in the buffer.
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
 * Compares the remote endpoint of the incoming packet with the stored endpoints
 * of connected clients. If a match is found, updates the client's last
 * heartbeat timestamp and returns the client's index. If no match is found,
 * returns KO.
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

  serialization::Buffer buffer(reinterpret_cast<const uint8_t *>(data),
                               reinterpret_cast<const uint8_t *>(data) + size);

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

void server::Server::clearClientSlot(int player_id) {
  for (auto &client : _clients) {
    if (client && client->_player_id == player_id) {
      client.reset();
      break;
    }
  }
}
