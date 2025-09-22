#pragma once

#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include "Component.hpp"

using ComponentType = std::uint8_t;

namespace ecs {

  class ComponentManager {
    public:
      ComponentManager() = default;
      ~ComponentManager() = default;

      template <typename T>
      void registerComponent() {
        std::type_index ti(typeid(T));
        if (_componentTypes.find(ti) != _componentTypes.end()) {
          throw std::runtime_error(
              "Cannot register component: Component type already registered.");
        }
        if (_nextComponentType >= MAX_COMPONENTS) {
          throw std::runtime_error(
              "Cannot register component: Maximum number of components "
              "reached.");
        }
        _componentTypes[ti] = _nextComponentType;
        _componentArrays[_nextComponentType] = std::make_shared<Component<T>>();
        _nextComponentType++;
      }

      template <typename T>
      ComponentType getComponentType() {
        std::type_index ti(typeid(T));
        return _componentTypes[ti];
      }

      template <typename T>
      void addComponent(Entity entityId, T component) {
        getComponentArray<T>()->insertData(entityId, component);
      }

      template <typename T>
      void removeComponent(Entity entityId) {
        getComponentArray<T>()->removeData(entityId);
      }

      template <typename T>
      T &getComponent(Entity entityId) {
        return getComponentArray<T>()->getData(entityId);
      }

      template <typename T>
      bool hasComponent(Entity entityId) {
        return getComponentArray<T>()->hasData(entityId);
      }

      void entityDestroyed(Entity entityId) {
        for (auto &pair : _componentArrays) {
          pair.second->entityDestroyed(entityId);
        }
      }

    private:
      std::unordered_map<std::type_index, ComponentType> _componentTypes;
      std::unordered_map<ComponentType, std::shared_ptr<IComponentArray>>
          _componentArrays;
      ComponentType _nextComponentType = 0;

      template <typename T>
      std::shared_ptr<Component<T>> getComponentArray() {
        std::type_index ti(typeid(T));
        if (_componentArrays.find(_componentTypes[ti]) ==
            _componentArrays.end()) {
          throw std::runtime_error(
              "Cannot get component array: Component type not registered.");
        }
        return std::static_pointer_cast<Component<T>>(
            _componentArrays[_componentTypes[ti]]);
      }
  };

}  // namespace ecs
