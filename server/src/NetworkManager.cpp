#include "NetworkManager.hpp"
#include <asio.hpp>
#include <iostream>

server::NetworkManager::NetworkManager(std::uint16_t port)
    : _socket(_io_context, asio::ip::udp::endpoint(asio::ip::udp::v4(), port)),
      _signals(_io_context, SIGINT, SIGTERM),
      _eventTimer(std::make_shared<asio::steady_timer>(_io_context)),
      _timeoutTimer(std::make_shared<asio::steady_timer>(_io_context)) {
}

void server::NetworkManager::registerClient(
    int id, const asio::ip::udp::endpoint &endpoint) {
  _clientEndpoints[id] = endpoint;
}

void server::NetworkManager::unregisterClient(int id) {
  _clientEndpoints.erase(id);
}

void server::NetworkManager::sendToClient(int id, const char *data,
                                          std::size_t size) {
  auto it = _clientEndpoints.find(id);
  if (it == _clientEndpoints.end())
    return;

  const auto &endpoint = it->second;
  _socket.async_send_to(
      asio::buffer(data, size), endpoint,
      [data](const asio::error_code &error, std::size_t bytes_sent) {
        if (error) {
          std::cerr << "[ERROR] Send failed: " << error.message() << std::endl;
        }
      });
}

void server::NetworkManager::sendToAll(const char *data, std::size_t size) {
  for (const auto &[id, endpoint] : _clientEndpoints) {
    _socket.async_send_to(
        asio::buffer(data, size), endpoint,
        [data](const asio::error_code &error, std::size_t bytes_sent) {
          if (error) {
            std::cerr << "[ERROR] Broadcast failed: " << error.message()
                      << std::endl;
          }
        });
  }
}

void server::NetworkManager::checkSignals() {
  _signals.async_wait(
      [this](const asio::error_code &error, [[maybe_unused]] int) {
        if (!error) {
          std::cout << "\nStopping server..." << std::endl;
          this->_isRunning = false;
          if (_stopCallback) {
            _stopCallback();
          }
          this->stop();
        }
      });
}

void server::NetworkManager::run() {
  checkSignals();
  _io_context.run();
}

void server::NetworkManager::startReceive(
    const std::function<void(const char *, std::size_t)> &callback) {
  _socket.async_receive_from(
      asio::buffer(_recv_buffer), _remote_endpoint,
      [this, callback](const asio::error_code &error,
                       std::size_t bytes_transferred) {
        if (!error && bytes_transferred > 0) {
          callback(_recv_buffer.data(), bytes_transferred);
        } else if (error) {
          if (error != asio::error::operation_aborted) {
            std::cerr << "[WARNING] Receive failed: " << error.message()
                      << std::endl;
          }
        }
        if (_isRunning) {
          startReceive(callback);
        }
      });
}

void server::NetworkManager::scheduleEventProcessing(
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

void server::NetworkManager::scheduleTimeout(
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

void server::NetworkManager::stop() {
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
