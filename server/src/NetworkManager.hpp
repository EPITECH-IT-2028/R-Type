#pragma once

#include <asio.hpp>
#include "Macros.hpp"

namespace server {

  class NetworkManager {
    public:
      NetworkManager(std::uint16_t port);
      ~NetworkManager() = default;

      void startReceive(
          const std::function<void(const char *, std::size_t)> &callback);
      void send(const char *data, std::size_t size);
      void scheduleEventProcessing(std::chrono::milliseconds interval,
                                   const std::function<void()> &callback);
      void scheduleTimeout(std::chrono::seconds interval,
                           const std::function<void()> &callback);
      void stop();
      
      void setStopCallback(const std::function<void()> &callback) {
        _stopCallback = callback;
      }

      bool getIsRunning() {
        return _isRunning;
      }

      void run();

      asio::error_code getLastError() const {
        return _socket.local_endpoint().address().is_unspecified()
                   ? asio::error_code(asio::error::not_connected)
                   : asio::error_code();
      }

      std::array<char, BUFFER_SIZE> getRecvBuffer() {
        return _recv_buffer;
      }

      asio::io_context &getIOContext() {
        return _io_context;
      }

      asio::ip::udp::socket &getSocket() {
        return _socket;
      }

      asio::ip::udp::endpoint &getRemoteEndpoint() {
        return _remote_endpoint;
      }

      asio::signal_set &getSignalSet() {
        return _signals;
      }

      void closeSocket() {
        _socket.close();
      }

      std::shared_ptr<asio::steady_timer> &getEventTimer() {
        return _eventTimer;
      }

      std::shared_ptr<asio::steady_timer> &getTimeoutTimer() {
        return _timeoutTimer;
      }

      void checkSignals();

    private:
      bool _isRunning = true;
      asio::io_context _io_context;
      asio::ip::udp::socket _socket;
      asio::ip::udp::endpoint _remote_endpoint;
      asio::signal_set _signals;
      std::shared_ptr<asio::steady_timer> _eventTimer;
      std::shared_ptr<asio::steady_timer> _timeoutTimer;
      bool _timeoutScheduled = false;
      std::array<char, BUFFER_SIZE> _recv_buffer;
      std::function<void()> _stopCallback;
  };

}  // namespace server
