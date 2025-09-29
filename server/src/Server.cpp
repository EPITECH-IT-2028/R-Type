#include "Server.hpp"
#include <cstring>
#include <iostream>
#include "Broadcast.hpp"
#include "IPacket.hpp"
#include "Macros.hpp"
#include "PacketSender.hpp"

server::Client::Client(const asio::ip::udp::endpoint &endpoint, int id)
    : _endpoint(endpoint), _player_id(id) {
  _connected = true;
  _last_heartbeat = std::chrono::steady_clock::now();
}

server::Server::Server(asio::io_context &io_context, std::uint16_t port,
                       std::uint16_t max_clients)
    : _io_context(io_context),
      _socket(io_context,
              asio::ip::udp::endpoint(asio::ip::udp::v4(),
                                      static_cast<unsigned short>(port))),
      _max_clients(max_clients),
      _port(port),
      _player_count(0),
      _projectile_count(0),
      _next_player_id(0),
      _next_projectile_id(0) {
  _clients.resize(max_clients);
}

void server::Server::start() {
  std::cout << "[CONSOLE] Server started on port " << _port << std::endl;
  _game.start();

  startReceive();

  _eventTimer = std::make_shared<asio::steady_timer>(_io_context);
  scheduleEventProcessing();

  _timeoutTimer = std::make_shared<asio::steady_timer>(_io_context);
  scheduleTimeoutCheck();
}

void server::Server::stop() {
  std::cout << "[CONSOLE] Server stopped..." << std::endl;
  if (_eventTimer) {
    _eventTimer->cancel();
  }

  if (_timeoutTimer) {
    _timeoutTimer->cancel();
  }

  _game.stop();
  _socket.close();
}

void server::Server::scheduleTimeoutCheck() {
  _timeoutTimer->expires_after(std::chrono::seconds(1));
  _timeoutTimer->async_wait([this](const asio::error_code &error) {
    if (!error) {
      handleTimeout();
      scheduleTimeoutCheck();
    }
  });
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
    if (duration.count() > 10) {
      const int pid = client->_player_id;
      const auto endpoint = client->_endpoint;
      std::cout << "[WORLD] Player " << pid << " timed out due to inactivity."
                << std::endl;
      client->_connected = false;
      if (_player_count > 0)
        --_player_count;

      if (auto player = _game.getPlayer(pid)) {
        _game.destroyPlayer(pid);
      }

      auto disconnectMsg = PacketBuilder::makeMessage(
          "Player " + std::to_string(pid) +
          " has been disconnected due to inactivity.");
      packet::PacketSender::sendPacket(_socket, endpoint, disconnectMsg);

      auto disconnectPacket = PacketBuilder::makePlayerDisconnect(pid);
      client.reset();
      broadcast::Broadcast::broadcastPlayerDisconnect(_socket, _clients,
                                                      disconnectPacket);
    }
  }
}

/*
 * This function create an event loop that processes game events at regular
 * intervals.
 */
void server::Server::scheduleEventProcessing() {
  _eventTimer->expires_after(std::chrono::milliseconds(50));
  _eventTimer->async_wait([this](const asio::error_code &error) {
    if (!error) {
      processGameEvents();
      scheduleEventProcessing();
    }
  });
}

void server::Server::processGameEvents() {
  queue::GameEvent event;

  while (_game.getEventQueue().popRequest(event)) {
    handleGameEvent(event);
  }
}

void server::Server::handleGameEvent(const queue::GameEvent &event) {
  std::visit(
      [this](const auto &specificEvent) {
        using T = std::decay_t<decltype(specificEvent)>;

        if constexpr (std::is_same_v<T, queue::EnemySpawnEvent>) {
          auto enemySpawnPacket = PacketBuilder::makeEnemySpawn(
              specificEvent.enemy_id, static_cast<EnemyType>(0x01),
              specificEvent.x, specificEvent.y, specificEvent.vx,
              specificEvent.vy, specificEvent.health, specificEvent.max_health);
          broadcast::Broadcast::broadcastEnemySpawn(_socket, _clients,
                                                    enemySpawnPacket);

        } else if constexpr (std::is_same_v<T, queue::EnemyDestroyEvent>) {
          auto enemyDeathPacket = PacketBuilder::makeEnemyDeath(
              specificEvent.enemy_id, specificEvent.x, specificEvent.y);
          broadcast::Broadcast::broadcastEnemyDeath(_socket, _clients,
                                                    enemyDeathPacket);

        } else if constexpr (std::is_same_v<T, queue::EnemyMoveEvent>) {
          auto enemyMovePacket = PacketBuilder::makeEnemyMove(
              specificEvent.enemy_id, specificEvent.x, specificEvent.y,
              specificEvent.vx, specificEvent.vy,
              specificEvent.sequence_number);
          broadcast::Broadcast::broadcastEnemyMove(_socket, _clients,
                                                   enemyMovePacket);
        } else if constexpr (std::is_same_v<T, queue::ProjectileSpawnEvent>) {
          auto projectileSpawnPacket = PacketBuilder::makeProjectileSpawn(
              specificEvent.projectile_id, specificEvent.type, specificEvent.x,
              specificEvent.y, specificEvent.vx, specificEvent.vy,
              specificEvent.is_enemy_projectile, specificEvent.damage,
              specificEvent.owner_id);
          broadcast::Broadcast::broadcastProjectileSpawn(_socket, _clients,
                                                         projectileSpawnPacket);
        }
      },
      event);
}

/*
 * Begin asynchronous receive operation.
 */
void server::Server::startReceive() {
  _socket.async_receive_from(
      asio::buffer(_recv_buffer), _remote_endpoint,
      [this](const asio::error_code &error, std::size_t bytes_transferred) {
        handleReceive(error, bytes_transferred);
      });
}

/*
 * Handle completion of a receive operation.
 */
void server::Server::handleReceive(const asio::error_code &error,
                                   std::size_t bytes_transferred) {
  if (error) {
    if (error == asio::error::operation_aborted) {
      return;
    }
    std::cerr << "[WARNING] Receive failed: " << error.message() << std::endl;
    startReceive();
    return;
  }
  if (bytes_transferred < sizeof(PacketHeader)) {
    std::cerr << "[DEBUG] Received packet too small: " << bytes_transferred
              << " bytes" << std::endl;
    startReceive();
    return;
  }

  int client_idx = findOrCreateClient(_remote_endpoint);
  if (client_idx == ERROR) {
    std::cerr << "[WARNING] Max clients reached. Refused connection."
              << std::endl;
    startReceive();
    return;
  } else {
    handleClientData(client_idx, _recv_buffer.data(), bytes_transferred);
  }
}

/**
 * Find an existing client by endpoint or create a new one if space allows.
 * Returns the index of the client in the _clients vector, or -1 if no space
 * is available.
 */
int server::Server::findOrCreateClient(
    const asio::ip::udp::endpoint &endpoint) {
  for (size_t i = 0; i < _clients.size(); ++i) {
    if (_clients[i] && _clients[i]->_endpoint == endpoint) {
      _clients[i]->_connected = true;
      return static_cast<int>(i);
    }
  }

  for (size_t i = 0; i < _clients.size(); ++i) {
    if (!_clients[i]) {
      _clients[i] = std::make_shared<Client>(endpoint, _next_player_id++);
      _player_count++;
      _clients[i]->_connected = true;
      std::cout << "[WORLD] New player connected with ID "
                << _clients[i]->_player_id << std::endl;
      auto msg = PacketBuilder::makeMessage("Welcome to the server!");
      packet::PacketSender::sendPacket(_socket, _clients[i]->_endpoint, msg);
      return static_cast<int>(i);
    }
  }

  return ERROR;
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
  PacketHeader header{};
  std::memcpy(&header, data, sizeof(PacketHeader));
  if (size < header.size || header.size < sizeof(PacketHeader)) {
    std::cerr << "[WARNING] Invalid packet size from client "
              << _clients[client_idx]->_player_id << std::endl;
    startReceive();
    return;
  }

  auto handler = _factory.createHandler(header.type);
  if (handler) {
    handler->handlePacket(*this, *client, data, header.size);
  } else {
    std::cerr << "[WARNING] Unknown packet type "
              << static_cast<int>(header.type) << " from client "
              << _clients[client_idx]->_player_id << std::endl;
  }
  startReceive();
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
