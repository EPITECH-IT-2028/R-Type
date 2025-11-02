#include "EntityManager.hpp"
#include <stdexcept>

ecs::EntityManager::EntityManager() {
  for (Entity entity = 0; entity < MAX_ENTITIES; ++entity) {
    _entities.push(entity);
  }
}

Entity ecs::EntityManager::createEntity() {
  if (_entities.empty())
    throw std::runtime_error("No entities available.");
  Entity entityId = _entities.front();
  _entities.pop();
  return entityId;
}

void ecs::EntityManager::destroyEntity(Entity entityId) {
  if (entityId >= MAX_ENTITIES)
    throw std::runtime_error("Entity ID out of range.");
  _signatures[entityId].reset();
  _entities.push(entityId);
}

void ecs::EntityManager::setSignature(Entity entityId, Signature signature) {
  if (entityId >= MAX_ENTITIES)
    throw std::runtime_error("Entity ID out of range.");
  _signatures[entityId] = signature;
}

Signature ecs::EntityManager::getSignature(Entity entityId) const {
  if (entityId >= MAX_ENTITIES)
    throw std::runtime_error("Entity ID out of range.");
  return _signatures[entityId];
}

std::vector<Entity> ecs::EntityManager::getAllEntities() const {
  std::vector<Entity> entities;
  for (Entity entity = 0; entity < MAX_ENTITIES; ++entity) {
    if (_signatures[entity].any()) {
      entities.push_back(entity);
    }
  }
  return entities;
}

bool ecs::EntityManager::isEntityValid(Entity entityId) const {
  if (entityId >= MAX_ENTITIES)
    return false;
  return _signatures[entityId].any();
}
