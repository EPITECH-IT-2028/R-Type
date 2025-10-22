#pragma once

#include <array>
#include <bitset>
#include <cstdint>
#include <queue>

constexpr int MAX_ENTITIES = 5000;
constexpr int MAX_COMPONENTS = 32;

using Entity = std::uint32_t;

using Signature = std::bitset<MAX_COMPONENTS>;

namespace ecs {

  class EntityManager {
    public:
      EntityManager();

      ~EntityManager() = default;

      Entity createEntity();
      void destroyEntity(Entity entityId);

      void setSignature(Entity entityId, Signature signature);
      Signature getSignature(Entity entityId) const;
      std::vector<Entity> getAllEntities() const;

    private:
      std::queue<Entity> _entities;
      std::array<Signature, MAX_ENTITIES> _signatures;
  };

}  // namespace ecs