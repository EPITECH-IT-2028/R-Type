#pragma once

#include <mutex>
#include "ComponentManager.hpp"
#include "EntityManager.hpp"
#include "SystemManager.hpp"

namespace ecs {
  class ECSManager {
    public:
      ECSManager()
          : _entityManager(std::make_unique<EntityManager>()),
            _componentManager(std::make_unique<ComponentManager>()),
            _systemManager(std::make_unique<SystemManager>()) {
      }
      ECSManager(const ECSManager &) = delete;
      ECSManager &operator=(const ECSManager &) = delete;
      ECSManager(ECSManager &&) = delete;
      ECSManager &operator=(ECSManager &&) = delete;
      ~ECSManager() = default;

      static ECSManager &getInstance() {
        static ECSManager instance;
        return instance;
      }

      Entity createEntity() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _entityManager->createEntity();
      }

      void destroyEntity(Entity entityId) {
        std::lock_guard<std::mutex> lock(_mutex);
        _componentManager->entityDestroyed(entityId);
        _systemManager->entityDestroyed(entityId);
        _entityManager->destroyEntity(entityId);
      }

      std::vector<Entity> getAllEntities() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _entityManager->getAllEntities();
      }

      int getEntityCount() {
        std::lock_guard<std::mutex> lock(_mutex);
        return getAllEntities().size();
      }

      template <typename T>
      bool hasComponent(Entity entityId) {
        std::lock_guard<std::mutex> lock(_mutex);
        return _componentManager->hasComponent<T>(entityId);
      }

      template <typename T>
      void registerComponent() {
        std::lock_guard<std::mutex> lock(_mutex);
        _componentManager->registerComponent<T>();
      }

      template <typename T>
      bool isComponentRegistered() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _componentManager->isComponentRegistered<T>();
      }

      template <typename T>
      void addComponent(Entity entityId, T component) {
        std::lock_guard<std::mutex> lock(_mutex);
        _componentManager->addComponent<T>(entityId, component);

        auto signature = _entityManager->getSignature(entityId);
        signature.set(_componentManager->getComponentType<T>(), true);
        _entityManager->setSignature(entityId, signature);
        _systemManager->entitySignatureChanged(entityId, signature);
      }

      template <typename T>
      void removeComponent(Entity entityId) {
        std::lock_guard<std::mutex> lock(_mutex);
        _componentManager->removeComponent<T>(entityId);

        auto signature = _entityManager->getSignature(entityId);
        signature.set(_componentManager->getComponentType<T>(), false);
        _entityManager->setSignature(entityId, signature);
        _systemManager->entitySignatureChanged(entityId, signature);
      }

      template <typename T>
      T &getComponent(Entity entityId) {
        std::lock_guard<std::mutex> lock(_mutex);
        return _componentManager->getComponent<T>(entityId);
      }

      template <typename T>
      ComponentType getComponentType() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _componentManager->getComponentType<T>();
      }

      template <typename T>
      std::shared_ptr<T> registerSystem() {
        std::lock_guard<std::mutex> lock(_mutex);
        auto system = _systemManager->registerSystem<T>();

        if constexpr (requires { system->setECSManager(this); })
          system->setECSManager(this);
        return system;
      }

      template <typename T>
      void setSystemSignature(Signature signature) {
        std::lock_guard<std::mutex> lock(_mutex);
        _systemManager->setSignature<T>(signature);
      }

      template <typename T>
      std::shared_ptr<T> getSystem() {
        std::lock_guard<std::mutex> lock(_mutex);
        return _systemManager->getSystem<T>();
      }

      void update(float dt) {
        _systemManager->update(dt);
      }

    private:
      std::unique_ptr<EntityManager> _entityManager;
      std::unique_ptr<ComponentManager> _componentManager;
      std::unique_ptr<SystemManager> _systemManager;
      mutable std::mutex _mutex;
  };
}  // namespace ecs
