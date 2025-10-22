#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>
#include "EntityManager.hpp"
#include "System.hpp"

namespace ecs {
  class SystemManager {
    public:
      template <typename T>
      std::shared_ptr<T> registerSystem() {
        std::type_index ti(typeid(T));
        if (_systems.find(ti) != _systems.end()) {
          throw std::runtime_error(
              "Cannot register system: System already registered.");
        }

        auto system = std::make_shared<T>();
        _systems.insert({ti, system});
        return system;
      }

      template <typename T>
      void setSignature(Signature signature) {
        std::type_index ti(typeid(T));
        _signatures.insert({ti, signature});
      }

      template <typename T>
      std::shared_ptr<T> getSystem() {
        std::type_index ti(typeid(T));
        auto it = _systems.find(ti);
        if (it != _systems.end()) {
          return std::static_pointer_cast<T>(it->second);
        }
        return nullptr;
      }

      void entityDestroyed(Entity entityId) {
        for (auto &pair : _systems) {
          pair.second->_entities.erase(entityId);
        }
      }

      void entitySignatureChanged(Entity entityId, Signature entitySignature) {
        for (auto const &pair : _systems) {
          auto const &type = pair.first;
          auto const &system = pair.second;
          auto const &systemSignature = _signatures[type];

          if ((entitySignature & systemSignature) == systemSignature) {
            system->_entities.insert(entityId);
          } else {
            system->_entities.erase(entityId);
          }
        }
      }

      void update(float dt) {
        for (auto const &[type, system] : _systems)
          system->update(dt);
      }

    private:
      std::unordered_map<std::type_index, std::shared_ptr<System>> _systems;
      std::unordered_map<std::type_index, Signature> _signatures;
  };
}  // namespace ecs
