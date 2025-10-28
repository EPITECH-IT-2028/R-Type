#include "ServerNetworkManager.hpp"
#include <iostream>
#include <memory>

using namespace network;

ServerNetworkManager::ServerNetworkManager(std::uint16_t port)
    : BaseNetworkManager(port),
      _isRunning(true),
      _signals(_io_context, SIGINT, SIGTERM),
      _eventTimer(std::make_shared<asio::steady_timer>(_io_context)),
      _timeoutTimer(std::make_shared<asio::steady_timer>(_io_context)),
      _unacknowledgedTimer(std::make_shared<asio::steady_timer>(_io_context)) {
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
  _signals.async_wait([this](const asio::error_code &error, int signal_number) {
    if (!error && _isRunning) {
      _isRunning = false;
      this->stop();
      _io_context.stop();
    }
  });
}

void ServerNetworkManager::stop() {
  if (!_isRunning) {
    return;
  }

  std::cout << "[CONSOLE] Stopping network manager..." << std::endl;
  _isRunning = false;

  _signals.cancel();

  if (_eventTimer) {
    _eventTimer->cancel();
  }
  if (_timeoutTimer) {
    _timeoutTimer->cancel();
  }
  if (_unacknowledgedTimer) {
    _unacknowledgedTimer->cancel();
  }
  if (_clearSeqTimer) {
    _clearSeqTimer->cancel();
  }

  closeSocket();

  if (_stopCallback) {
    _stopCallback();
  }

  std::cout << "[CONSOLE] Network manager stopped completely." << std::endl;
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
          if (_isRunning)
            std::cerr << "[WARNING] Receive failed: " << error.message()
                      << std::endl;
        }
        if (_isRunning && _socket.is_open())
          startReceive(callback);
      });
}

void ServerNetworkManager::scheduleEventProcessing(
    std::chrono::milliseconds interval, const std::function<void()> &callback) {
  if (_eventScheduled || !_isRunning)
    return;
  _eventScheduled = true;

  if (!_eventTimer) {
    _eventTimer = std::make_shared<asio::steady_timer>(_io_context);
  }

  _eventTimer->expires_after(interval);
  _eventTimer->async_wait(
      [this, interval, callback](const asio::error_code &error) {
        _eventScheduled = false;
        if (!error && _isRunning) {
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
        if (!error && _isRunning) {
          callback();
          scheduleTimeout(interval, callback);
        }
      });
}

void ServerNetworkManager::scheduleUnacknowledgedPacketsCheck(
    std::chrono::milliseconds interval, const std::function<void()> &callback) {
  if (_unacknowledgedScheduled)
    return;
  _unacknowledgedScheduled = true;
  _unacknowledgedTimer->expires_after(interval);
  _unacknowledgedTimer->async_wait(
      [this, interval, callback](const asio::error_code &error) {
        _unacknowledgedScheduled = false;
        if (!error && _isRunning) {
          callback();
          scheduleUnacknowledgedPacketsCheck(interval, callback);
        }
      });
}

void ServerNetworkManager::scheduleClearLastProcessedSeq(
    std::chrono::seconds interval, const std::function<void()> &callback) {
  if (_clearSeqScheduled || !_isRunning)
    return;
  _clearSeqScheduled = true;
  if (!_clearSeqTimer) {
    _clearSeqTimer = std::make_shared<asio::steady_timer>(_io_context);
  }
  _clearSeqTimer->expires_after(interval);
  _clearSeqTimer->async_wait(
      [this, interval, callback](const asio::error_code &error) {
        _clearSeqScheduled = false;
        if (!error && _isRunning) {
          callback();
          scheduleClearLastProcessedSeq(interval, callback);
        }
      });
}
