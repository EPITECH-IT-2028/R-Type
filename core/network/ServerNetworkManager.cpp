#include "ServerNetworkManager.hpp"
#include <iostream>

using namespace network;

ServerNetworkManager::ServerNetworkManager(std::uint16_t port)
    : BaseNetworkManager(port),
      _signals(_io_context, SIGINT, SIGTERM),
      _eventTimer(std::make_shared<asio::steady_timer>(_io_context)),
      _timeoutTimer(std::make_shared<asio::steady_timer>(_io_context)) {
}

void ServerNetworkManager::registerClient(
    int id, const asio::ip::udp::endpoint &endpoint) {
  _clientEndpoints[id] = endpoint;
}

void ServerNetworkManager::unregisterClient(int id) {
  _clientEndpoints.erase(id);
}

void ServerNetworkManager::sendToClient(int id, const char *data,
                                        std::size_t size) {
  auto buffer = std::make_shared<std::vector<std::uint8_t>>(data, data + size);
  sendToClient(id, buffer);
}

void ServerNetworkManager::sendToClient(
    int id, std::shared_ptr<std::vector<std::uint8_t>> buffer) {
  auto it = _clientEndpoints.find(id);
  if (it == _clientEndpoints.end())
    return;
  const auto &endpoint = it->second;
  _socket.async_send_to(
      asio::buffer(*buffer), endpoint,
      [buffer](const asio::error_code &error, std::size_t) {
        if (error)
          std::cerr << "[ERROR] Send failed: " << error.message() << std::endl;
      });
}

void ServerNetworkManager::run() {
  checkSignals();
  _io_context.run();
}

void ServerNetworkManager::sendToAll(const char *data, std::size_t size) {
  auto buffer = std::make_shared<std::vector<std::uint8_t>>(data, data + size);
  sendToAll(buffer);
}

void ServerNetworkManager::sendToAll(
    std::shared_ptr<std::vector<std::uint8_t>> buffer) {
  for (const auto &[id, endpoint] : _clientEndpoints) {
    _socket.async_send_to(asio::buffer(*buffer), endpoint,
                          [buffer](const asio::error_code &error, std::size_t) {
                            if (error)
                              std::cerr << "[ERROR] Broadcast failed: "
                                        << error.message() << std::endl;
                          });
  }
}

void ServerNetworkManager::send(const char *data, std::size_t size) {
  sendToAll(data, size);
}

void ServerNetworkManager::send(
    std::shared_ptr<std::vector<std::uint8_t>> buffer) {
  sendToAll(buffer);
}

void ServerNetworkManager::checkSignals() {
  _signals.async_wait([this](const asio::error_code &error, int) {
    if (!error) {
      std::cout << "\nStopping server..." << std::endl;
      _isRunning = false;
      if (_stopCallback)
        _stopCallback();
      this->stop();
    }
  });
}

void ServerNetworkManager::stop() {
  _isRunning = false;

  if (_eventTimer) {
    _eventTimer->cancel();
  }

  if (_timeoutTimer) {
    _timeoutTimer->cancel();
  }

  if (_stopCallback) {
    _stopCallback();
  }

  _io_context.stop();
}

void ServerNetworkManager::startReceive(
    const std::function<void(const char *, std::size_t)> &callback) {
  _socket.async_receive_from(
      asio::buffer(_recv_buffer), _remote_endpoint,
      [this, callback](const asio::error_code &error,
                       std::size_t bytes_transferred) {
        if (!error && bytes_transferred > 0) {
          callback(_recv_buffer.data(), bytes_transferred);
        } else if (error && error != asio::error::operation_aborted) {
          std::cerr << "[WARNING] Receive failed: " << error.message()
                    << std::endl;
        }
        if (_isRunning)
          startReceive(callback);
      });
}

void ServerNetworkManager::scheduleEventProcessing(
    std::chrono::milliseconds interval, const std::function<void()> &callback) {
  auto timer = std::make_shared<asio::steady_timer>(_io_context, interval);
  timer->async_wait(
      [this, timer, interval, callback](const asio::error_code &error) {
        if (!error) {
          callback();
          scheduleEventProcessing(interval, callback);
        }
      });
}

void ServerNetworkManager::scheduleTimeout(
    std::chrono::seconds interval, const std::function<void()> &callback) {
  if (_timeoutScheduled)
    return;
  _timeoutScheduled = true;
  _timeoutTimer->expires_after(interval);
  _timeoutTimer->async_wait(
      [this, interval, callback](const asio::error_code &error) {
        _timeoutScheduled = false;
        if (!error) {
          callback();
          scheduleTimeout(interval, callback);
        }
      });
}
