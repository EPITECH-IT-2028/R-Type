#pragma once

#include <mutex>
#include <set>
#include "EntityManager.hpp"

namespace ecs {

  class System {
    public:
      std::set<Entity> _entities;
      mutable std::mutex _mutex;
      virtual ~System() = default;
      virtual void update(float deltaTime) = 0;
  };

}  // namespace ecs
