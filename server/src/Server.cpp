#include "Server.hpp"
#include <cstring>
#include <iostream>
#include "IPacket.hpp"
#include "Macros.hpp"
#include "PacketBuilder.hpp"
#include "PacketSender.hpp"

server::Client::Client(const asio::ip::udp::endpoint &endpoint, int id)
    : _endpoint(endpoint), _player_id(id) {
  _connected = true;
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
      _next_player_id(0) {
  _clients.resize(max_clients);
}

void server::Server::start() {
  std::cout << "[CONSOLE] Server started on port " << _port << std::endl;
  _game.start();
  startReceive();
}

void server::Server::stop() {
  std::cout << "[CONSOLE] Server stopped..." << std::endl;
  _socket.close();
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
