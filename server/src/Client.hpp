#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <limits>
#include "Macro.hpp"

namespace server {
  struct Client {
    public:
      /**
       * @brief Constructs a Client for the given player identifier and
       * initializes connection state and timestamps.
       *
       * Initializes the client with the provided player id, marks it as
       * connected, assigns no room, and records the current steady-clock time
       * for both the last heartbeat and last position update.
       *
       * @param id Player identifier to associate with this client.
       */
      Client(int id)
          : _player_id(id),
            _connected(true),
            _room_id(NO_ROOM),
            _last_heartbeat(std::chrono::steady_clock::now()),
            _last_position_update(std::chrono::steady_clock::now()) {
      }
      /**
       * @brief Destroys the Client object.
       *
       * Performs default destruction of the client's members.
       */
      ~Client() = default;

      bool _connected = false;
      int _player_id = -1;
      uint32_t _room_id = NO_ROOM;
      std::chrono::steady_clock::time_point _last_heartbeat;
      std::chrono::steady_clock::time_point _last_position_update;
      uint32_t _entity_id = std::numeric_limits<uint32_t>::max();
  };
}  // namespace server
