
#pragma once

#include <mutex>
#include <queue>
#include "Events.hpp"

namespace queue {

  class EventQueue {
    public:
      EventQueue() = default;
      ~EventQueue() = default;

      void addRequest(const GameEvent &event) {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push(event);
      }

      bool popRequest(GameEvent &event) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_queue.empty()) {
          return false;
        }
        event = _queue.front();
        _queue.pop();
        return true;
      }

    private:
      std::queue<GameEvent> _queue;
      mutable std::mutex _mutex;
  };

}  // namespace queue
