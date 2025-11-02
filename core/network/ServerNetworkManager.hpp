#pragma once

#include <chrono>
#include <csignal>
#include <cstdint>
#include <memory>
#include <unordered_map>
#include "BaseNetworkManager.hpp"

namespace network {

  class ServerNetworkManager : public BaseNetworkManager {
    public:
      explicit ServerNetworkManager(std::uint16_t port);

      void startReceive(const std::function<void(const char *, std::size_t)>
                            &callback) override;
      void send(const char *data, std::size_t size) override;
      void send(std::shared_ptr<std::vector<std::uint8_t>> buffer) override;

      void registerClient(int id, const asio::ip::udp::endpoint &endpoint);
      void unregisterClient(int id);

      void sendToClient(int id, const char *data, std::size_t size);
      void sendToClient(int id,
                        std::shared_ptr<std::vector<std::uint8_t>> buffer);
      void sendToAll(const char *data, std::size_t size);
      void sendToAll(std::shared_ptr<std::vector<std::uint8_t>> buffer);

      void scheduleEventProcessing(std::chrono::milliseconds interval,
                                   const std::function<void()> &callback);
      void scheduleTimeout(std::chrono::seconds interval,
                           const std::function<void()> &callback);
      void scheduleUnacknowledgedPacketsCheck(
          std::chrono::milliseconds interval,
          const std::function<void()> &callback);

      void scheduleClearLastProcessedSeq(std::chrono::seconds interval,
                                         const std::function<void()> &callback);

      void checkSignals();

      /**
       * @brief Sets a callback to be invoked when the server is stopped.
       *
       * The provided callback will be stored and called during shutdown/stop
       * processing.
       *
       * @param callback Function to call when the server stops.
       */
      void setStopCallback(const std::function<void()> &callback) {
        _stopCallback = callback;
      }

      void run() override;

      void stop() override;

      /**
       * @brief Cancels pending operations and closes the underlying socket if
       * it is open.
       *
       * If the socket is open, this will cancel any outstanding asynchronous
       * operations and then close the socket. If the socket is already closed,
       * the call has no effect.
       */
      void closeSocket() {
        if (_socket.is_open()) {
          _socket.cancel();
          _socket.close();
        }
      };

      /**
       * @brief Retrieve the UDP endpoint associated with a player ID.
       *
       * @param player_id Player identifier used as the lookup key.
       * @return asio::ip::udp::endpoint The stored UDP endpoint for the given
       * player.
       * @throws std::out_of_range If no endpoint is registered for `player_id`.
       */
      asio::ip::udp::endpoint getClientEndpoint(std::uint32_t player_id) {
        return _clientEndpoints.at(player_id);
      }

      asio::ip::udp::endpoint getRemoteEndpoint() {
        return _remote_endpoint;
      }

    private:
      asio::ip::udp::endpoint _remote_endpoint;
      asio::signal_set _signals;
      std::shared_ptr<asio::steady_timer> _eventTimer;
      std::shared_ptr<asio::steady_timer> _timeoutTimer;
      std::shared_ptr<asio::steady_timer> _unacknowledgedTimer;
      std::shared_ptr<asio::steady_timer> _clearSeqTimer;
      std::unordered_map<int, asio::ip::udp::endpoint> _clientEndpoints;
      std::function<void()> _stopCallback;
      std::atomic<bool> _isRunning = false;
      bool _unacknowledgedScheduled = false;
      bool _timeoutScheduled = false;
      bool _eventScheduled = false;
      bool _clearSeqScheduled = false;
  };

}  // namespace network
