#include "ServerNetworkManager.hpp"
#include <iostream>
#include <memory>

using namespace network;

/**
 * @brief Constructs a ServerNetworkManager bound to a specific port.
 *
 * Initializes the server manager and prepares internal signal handling and timers needed for running the network service.
 *
 * @param port UDP port number the server will listen on.
 */
ServerNetworkManager::ServerNetworkManager(std::uint16_t port)
    : BaseNetworkManager(port),
      _isRunning(true),
      _signals(_io_context, SIGINT, SIGTERM),
      _eventTimer(std::make_shared<asio::steady_timer>(_io_context)),
      _timeoutTimer(std::make_shared<asio::steady_timer>(_io_context)),
      _unacknowledgedTimer(std::make_shared<asio::steady_timer>(_io_context)) {
}

/**
 * @brief Associate a client identifier with its UDP endpoint for future communication.
 *
 * Stores or updates the mapping from the given client `id` to the provided UDP `endpoint`.
 *
 * @param id Client identifier used as the key for the mapping.
 * @param endpoint UDP endpoint (address and port) associated with the client.
 */
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

/**
 * @brief Sends the provided byte buffer to all registered clients.
 *
 * @param buffer Shared pointer to a vector of bytes containing the payload to send to every connected client.
 */
void ServerNetworkManager::send(
    std::shared_ptr<std::vector<std::uint8_t>> buffer) {
  sendToAll(buffer);
}

/**
 * @brief Registers asynchronous OS signal handlers that initiate an orderly shutdown.
 *
 * Sets up an asynchronous wait on the internal signal set so that when a registered
 * signal is received and the server is running, the manager is marked not running,
 * stop() is invoked, and the io_context is stopped.
 */
void ServerNetworkManager::checkSignals() {
  _signals.async_wait([this](const asio::error_code &error, int signal_number) {
    if (!error && _isRunning) {
      _isRunning = false;
      this->stop();
      _io_context.stop();
    }
  });
}

/**
 * @brief Stops the network manager and performs full shutdown cleanup.
 *
 * Sets the manager as not running, cancels signal handlers and any active timers,
 * closes the network socket, and invokes the optional stop callback if provided.
 * If the manager is already stopped, this function returns without performing any actions.
 */
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

/**
 * @brief Begins and continuously receives datagrams from the socket, invoking the provided callback for each received packet.
 *
 * The function keeps posting receive operations while the manager is running and the socket remains open.
 * On a successful receive with data, the callback is invoked with a pointer to the received bytes and the number of bytes.
 * Receive errors other than `asio::error::operation_aborted` are logged as warnings when the manager is running.
 *
 * @param callback Function invoked for each received packet; receives a pointer to the data buffer and the number of bytes received.
 */
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

/**
 * @brief Schedules a recurring event-processing callback to run at the given interval.
 *
 * If an event is already scheduled or the server is not running, this call has no effect.
 * The provided callback is invoked only when the server remains running; after each successful
 * invocation the timer re-schedules itself to continue periodic execution.
 *
 * @param interval Time between callback invocations.
 * @param callback Function to execute on each scheduled event.
 */
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

/**
 * @brief Schedules a recurring timeout callback at the specified interval.
 *
 * If a timeout is already scheduled, this call has no effect. When the timer
 * expires and the server is running, the provided callback is invoked and the
 * timeout is re-scheduled with the same interval.
 *
 * @param interval Delay between consecutive callback invocations.
 * @param callback Function to invoke when the timer expires.
 */
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

/**
 * @brief Schedules recurring checks for unacknowledged packets at a fixed interval.
 *
 * If a check is already scheduled, this call has no effect. When the timer expires
 * and the manager is running, the provided callback is invoked and the check is
 * rescheduled to run again after the same interval.
 *
 * @param interval Time between consecutive checks.
 * @param callback Function to call when a check is due.
 */
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

/**
 * @brief Schedule repeated execution of a callback to clear the last processed sequence.
 *
 * Sets up an internal timer to invoke the provided callback every given interval while the
 * server remains running. If a clear-sequence timer is already scheduled or the manager is
 * not running, the call is a no-op. The timer re-schedules itself after each successful
 * invocation.
 *
 * @param interval Duration between consecutive callback invocations.
 * @param callback Function invoked to clear the last processed sequence when the timer expires.
 */
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