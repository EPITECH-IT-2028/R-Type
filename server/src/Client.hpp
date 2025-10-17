#pragma once

#include <asio.hpp>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include "Macro.hpp"

namespace server {
  struct Client {
    public:
      Client(int id) : _player_id(id) {
        _connected = true;
        _last_heartbeat = std::chrono::steady_clock::now();
        _last_position_update = std::chrono::steady_clock::now();
        _room_id = NO_ROOM;
      }
      ~Client() = default;

      bool _connected = false;
      int _player_id = -1;
      uint32_t _room_id = NO_ROOM;
      std::chrono::steady_clock::time_point _last_heartbeat;
      std::chrono::steady_clock::time_point _last_position_update;
      uint32_t _entity_id = std::numeric_limits<uint32_t>::max();
  };
}  // namespace server
