# Game Engine (ECS)

This section describes the Entity-Component-System (ECS) architecture implemented in the R-Type game engine. The ECS pattern promotes a clear separation of concerns, leading to more flexible, scalable, and maintainable game logic.

## Core Concepts

The R-Type ECS is built around three fundamental concepts:

*   **Entities**: Represent unique game objects (e.g., a player, an enemy, a projectile). An entity is essentially just a unique ID.
*   **Components**: Raw data associated with an entity (e.g., `PositionComponent`, `HealthComponent`, `SpriteComponent`). Components contain no logic; they only store data.
*   **Systems**: Logic that operates on entities that possess a specific set of components (e.g., `MovementSystem` operates on entities with `PositionComponent` and `VelocityComponent`). Systems perform actions and modify component data.

## Architecture Overview

The ECS implementation consists of several key managers:

### `ECSManager`

The `ECSManager` is the central orchestrator of the entire ECS. It provides the main interface for interacting with the ECS, abstracting away the complexities of the underlying managers. It is responsible for:

*   Creating and destroying entities.
*   Registering component types.
*   Adding and removing components from entities.
*   Retrieving components from entities.
*   Registering and setting signatures for systems.
*   Triggering the update cycle for all registered systems.

### `EntityManager`

The `EntityManager` is responsible for managing the lifecycle of entities. It:

*   Assigns unique IDs to new entities.
*   Recycles IDs of destroyed entities.
*   Maintains a `Signature` for each entity, which is a bitset indicating which components are attached to that entity.

### `ComponentManager`

The `ComponentManager` handles the registration and storage of different component types. Key features include:

*   **Component Registration**: Allows new component types to be registered with the ECS.
*   **Data-Oriented Storage**: Components of the same type are stored contiguously in memory using `std::array<T, MAX_ENTITIES>` within `ecs::Component<T>`, improving cache performance.
*   **Component Operations**: Provides methods to add, remove, and retrieve components for specific entities.

### `SystemManager`

The `SystemManager` is responsible for managing all the systems in the ECS. It:

*   **System Registration**: Allows new systems to be registered.
*   **System Signatures**: Each system has a `Signature` that defines the set of components an entity must possess for the system to process it. When an entity's components change, the `SystemManager` updates which entities each system is interested in.
*   **System Updates**: Iterates through all registered systems and calls their `update` method, passing the `deltaTime`.

### `System` Base Class

All systems inherit from the `ecs::System` base class. A system typically:

*   Maintains a `std::set<Entity>` of entities that match its signature.
*   Implements an `update(float deltaTime)` method where its logic is executed. This method iterates over the entities in its set and performs operations based on their components.

## How it Works Together

1.  **Initialization**: The `ECSManager` is initialized, which in turn initializes the `EntityManager`, `ComponentManager`, and `SystemManager`.
2.  **Entity Creation**: When `createEntity()` is called, the `EntityManager` provides a new unique ID.
3.  **Component Attachment**: When `addComponent()` is called, the `ComponentManager` stores the component data, and the `EntityManager` updates the entity's `Signature`. The `SystemManager` is then notified to update which systems are interested in this entity.
4.  **System Logic**: During each game loop iteration, `ECSManager::update()` calls `SystemManager::update()`, which then calls the `update()` method of each registered system. Each system then processes the entities that match its signature.
5.  **Entity Destruction**: When `destroyEntity()` is called, the `ECSManager` orchestrates the removal of all components associated with the entity and notifies systems to remove the entity from their processing sets.

This modular design allows for easy addition of new game features by simply creating new components and systems without modifying existing code.