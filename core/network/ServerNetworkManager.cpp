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

/**
 * @brief Send raw bytes to a registered client identified by its id.
 *
 * @param id Identifier of the target client whose endpoint was previously registered.
 * @param data Pointer to the buffer containing the bytes to send.
 * @param size Number of bytes to send from `data`.
 */
void ServerNetworkManager::sendToClient(int id, const char *data,
                                        std::size_t size) {
  auto buffer = std::make_shared<std::vector<std::uint8_t>>(data, data + size);
  sendToClient(id, buffer);
}

/**
 * @brief Asynchronously sends the provided byte buffer to the client with the given id.
 *
 * If the client id is not registered, the function returns without performing any send.
 * The provided `buffer` is captured and held for the duration of the asynchronous send.
 * On send failure, an error message is written to `std::cerr`.
 *
 * @param id Target client identifier.
 * @param buffer Shared pointer to the byte vector to send; ownership is retained until the async operation completes.
 */
void ServerNetworkManager::sendToClient(
    int id, std::shared_ptr<std::vector<std::uint8_t>> buffer) {
  auto it = _clientEndpoints.find(id);
  if (it == _clientEndpoints.end())
    return;
  const auto &endpoint = it->second;
  _socket.async_send_to(
      asio::buffer(*buffer), endpoint,
      [buffer](const asio::error_code &error, std::size_t) {  // Capturer buffer
        if (error)
          std::cerr << "[ERROR] Send failed: " << error.message() << std::endl;
      });
}

/**
 * @brief Starts the network manager: sets up signal handling and begins processing IO events.
 *
 * This call sets up the process signal handlers and runs the ASIO I/O context; it blocks
 * until the I/O context is stopped (e.g., by a signal or by calling stop()).
 */
void ServerNetworkManager::run() {
  checkSignals();
  _io_context.run();
}

/**
 * @brief Broadcasts raw bytes to all currently registered clients.
 *
 * Copies the provided data into an internal shared buffer and initiates asynchronous
 * sends to every known client endpoint; the buffer's lifetime is preserved until
 * the asynchronous operations complete.
 *
 * @param data Pointer to the raw data to send.
 * @param size Number of bytes to send from `data`.
 */
void ServerNetworkManager::sendToAll(const char *data, std::size_t size) {
  auto buffer = std::make_shared<std::vector<std::uint8_t>>(data, data + size);
  sendToAll(buffer);
}

/**
 * @brief Asynchronously sends the same byte buffer to all registered clients.
 *
 * The provided buffer is held by each asynchronous send operation to ensure
 * the data remains valid until transmission completes. Failures for
 * individual sends are reported to standard error.
 *
 * @param buffer Shared ownership of the bytes to broadcast to every client.
 */
void ServerNetworkManager::sendToAll(
    std::shared_ptr<std::vector<std::uint8_t>> buffer) {
  for (const auto &[id, endpoint] : _clientEndpoints) {
    _socket.async_send_to(asio::buffer(*buffer), endpoint,
                          [buffer](const asio::error_code &error,
                                   std::size_t) {  // Capturer buffer
                            if (error)
                              std::cerr << "[ERROR] Broadcast failed: "
                                        << error.message() << std::endl;
                          });
  }
}

/**
 * @brief Sends a raw byte sequence to all connected clients.
 *
 * @param data Pointer to the buffer containing the bytes to send.
 * @param size Number of bytes to send from `data`.
 */
void ServerNetworkManager::send(const char *data, std::size_t size) {
  sendToAll(data, size);
}

/**
 * @brief Sends the provided byte buffer to all registered clients.
 *
 * The buffer is shared to allow the data to remain valid for any asynchronous
 * send operations initiated by the method.
 *
 * @param buffer Shared pointer to the byte vector to broadcast to every client.
 */
void ServerNetworkManager::send(
    std::shared_ptr<std::vector<std::uint8_t>> buffer) {
  sendToAll(buffer);
}

/**
 * @brief Registers a handler that stops the server when a termination signal is received.
 *
 * Sets up a signal handler that, when a signal is delivered without error, prints "Stopping server...",
 * sets the running flag to false, invokes the stop callback if one is configured, and stops the server.
 */
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