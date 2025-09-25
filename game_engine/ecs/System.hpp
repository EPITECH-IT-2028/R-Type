#pragma once

#include "EntityManager.hpp"
#include <set>

namespace ecs {

  class System {
    public:
      std::set<Entity> _entities;
      virtual ~System() = default;
      virtual void update(float deltaTime) = 0;
  };

}
