#pragma once

#include <array>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <unordered_map>
#include "EntityManager.hpp"

namespace ecs {

  class IComponentArray {
    public:
      virtual ~IComponentArray() = default;
      virtual void entityDestroyed(Entity entityId) = 0;
  };

  template <typename T>
  class Component : public IComponentArray {
    public:
      void insertData(Entity entityId, T component) {
        if (_entityToIndexMap.find(entityId) != _entityToIndexMap.end()) {
          throw std::runtime_error(
              "Cannot insert component: Entity already has this component.");
        }
        _entityToIndexMap[entityId] = _index;
        _indexToEntityMap[_index] = entityId;
        _componentArray[_index] = component;
        _index++;
      }

      /**
       * @brief Remove the component for an entity and keep component storage dense.
       *
       * Deletes the component associated with the given entity, replaces the removed
       * slot with the last stored component to maintain a contiguous array, updates
       * internal entity/index mappings accordingly, and decrements the component count.
       *
       * @param entityId The entity whose component should be removed.
       * @throws std::runtime_error if the entity does not have this component.
       */
      void removeData(Entity entityId) {
        if (_entityToIndexMap.find(entityId) == _entityToIndexMap.end()) {
          throw std::runtime_error(
              "Cannot remove component: Entity does not have this component.");
        }
        size_t removedIndex = _entityToIndexMap[entityId];
        size_t lastIndex = _index - 1;
        _componentArray[removedIndex] = _componentArray[lastIndex];

        Entity lastEntity = _indexToEntityMap[lastIndex];
        _entityToIndexMap[lastEntity] = removedIndex;
        _indexToEntityMap[removedIndex] = lastEntity;

        _entityToIndexMap.erase(entityId);
        _indexToEntityMap.erase(lastIndex);
        --_index;
      }

      T &getData(Entity entityId) {
        if (_entityToIndexMap.find(entityId) == _entityToIndexMap.end()) {
          throw std::runtime_error(
              "Cannot get component: Entity does not have this component.");
        }
        return _componentArray[_entityToIndexMap[entityId]];
      }

      bool hasData(Entity entityId) const {
        return _entityToIndexMap.find(entityId) != _entityToIndexMap.end();
      }

      void entityDestroyed(Entity entityId) override {
        if (_entityToIndexMap.find(entityId) != _entityToIndexMap.end()) {
          removeData(entityId);
        }
      }

    private:
      std::array<T, MAX_ENTITIES> _componentArray;
      std::unordered_map<Entity, size_t> _entityToIndexMap;
      std::unordered_map<size_t, Entity> _indexToEntityMap;
      size_t _index = 0;
  };

}  // namespace ecs