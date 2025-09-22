#pragma once

#include "EntityManager.hpp"
#include "ComponentManager.hpp"
#include "SystemManager.hpp"

namespace ecs {
    class ECSManager {
      public:
        ECSManager()
            : _entityManager(std::make_unique<EntityManager>()),
              _componentManager(std::make_unique<ComponentManager>()),
              _systemManager(std::make_unique<SystemManager>()) {}

        ~ECSManager() = default;

        Entity createEntity() {
          return _entityManager->createEntity();
        }

        void destroyEntity(Entity entityId) {
          _entityManager->destroyEntity(entityId);
          _componentManager->entityDestroyed(entityId);
          _systemManager->entityDestroyed(entityId);
        }

        std::vector<Entity> getAllEntities() {
          return _entityManager->getAllEntities();
        }

        int getEntityCount() {
          return getAllEntities().size();
        }

        template <typename T>
        bool hasComponent(Entity entityId) {
          return _componentManager->hasComponent<T>(entityId);
        }

        template <typename T>
        void registerComponent() {
          _componentManager->registerComponent<T>();
        }

        template <typename T>
        void addComponent(Entity entityId, T component) {
          _componentManager->addComponent<T>(entityId, component);

          auto signature = _entityManager->getSignature(entityId);
          signature.set(_componentManager->getComponentType<T>(), true);
          _entityManager->setSignature(entityId, signature);
          _systemManager->entitySignatureChanged(entityId, signature);
        }

        template <typename T>
        void removeComponent(Entity entityId) {
          _componentManager->removeComponent<T>(entityId);

          auto signature = _entityManager->getSignature(entityId);
          signature.set(_componentManager->getComponentType<T>(), false);
          _entityManager->setSignature(entityId, signature);
          _systemManager->entitySignatureChanged(entityId, signature);
        }

        template <typename T>
        T &getComponent(Entity entityId) {
          return _componentManager->getComponent<T>(entityId);
        }

        template <typename T>
        ComponentType getComponentType() {
          return _componentManager->getComponentType<T>();
        }

        template <typename T>
        std::shared_ptr<T> registerSystem() {
          return _systemManager->registerSystem<T>();
        }

        template <typename T>
        void setSystemSignature(Signature signature) {
          _systemManager->setSignature<T>(signature);
        }

        template <typename T>
        std::shared_ptr<T> getSystem() {
          return _systemManager->getSystem<T>();
        }

      private:
        std::unique_ptr<EntityManager> _entityManager;
        std::unique_ptr<ComponentManager> _componentManager;
        std::unique_ptr<SystemManager> _systemManager;
    };
}