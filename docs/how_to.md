## 🧩 ECS Development Guide

Our R-Type project uses a custom Entity-Component-System (ECS) architecture. This guide explains how to extend the system with new components, systems, entities, and network packets.

### 🔧 Adding a New Component

Components are simple data structures that hold state for entities.

**1. Create the component header file:**
```cpp
// game_engine/ecs/components/NewComponent.hpp
#pragma once

namespace ecs {
  struct NewComponent {
    // Add your data members here
    float value = 0.0f;
    int count = 0;
    bool isActive = true;
  };
}  // namespace ecs
```

**2. Register the component in your initialization code:**
```cpp
// In your game initialization
auto& ecsManager = ecs::ECSManager::getInstance();
ecsManager.registerComponent<ecs::NewComponent>();
```

**3. Use the component:**
```cpp
// Add component to entity
Entity entity = ecsManager.createEntity();
ecsManager.addComponent(entity, NewComponent{1.5f, 10, true});

// Get and modify component
auto& component = ecsManager.getComponent<NewComponent>(entity);
component.value = 2.0f;

// Check if entity has component
if (ecsManager.hasComponent<NewComponent>(entity)) {
  // Do something
}

// Remove component
ecsManager.removeComponent<NewComponent>(entity);
```

### ⚙️ Adding a New System

Systems contain the logic that operates on entities with specific components.

**1. Create the system header file:**
```cpp
// game_engine/ecs/systems/NewSystem.hpp
#pragma once

#include "ECSManager.hpp"
#include "System.hpp"

namespace ecs {
  class NewSystem : public System {
    public:
      explicit NewSystem(ECSManager& ecsManager = ECSManager::getInstance())
          : _ecsManager(ecsManager) {}

      void update(float deltaTime) override;

    private:
      ECSManager& _ecsManager;
      
      // Add private methods for system logic
      void processEntity(Entity entity, float deltaTime);
  };
}  // namespace ecs
```

**2. Implement the system logic:**
```cpp
// game_engine/ecs/systems/NewSystem.cpp
#include "NewSystem.hpp"
#include "NewComponent.hpp"
#include "PositionComponent.hpp" // If you need other components

namespace ecs {
  void NewSystem::update(float deltaTime) {
    for (auto const& entity : _entities) {
      processEntity(entity, deltaTime);
    }
  }

  void NewSystem::processEntity(Entity entity, float deltaTime) {
    // Get required components
    auto& newComp = _ecsManager.getComponent<NewComponent>(entity);
    auto& position = _ecsManager.getComponent<PositionComponent>(entity);
    
    // Implement your system logic here
    if (newComp.isActive) {
      position.x += newComp.value * deltaTime;
      newComp.count++;
    }
  }
}  // namespace ecs
```

**3. Register and configure the system:**
```cpp
// In your game initialization
auto& ecsManager = ecs::ECSManager::getInstance();

// Register the system
auto newSystem = ecsManager.registerSystem<NewSystem>();

// Define which components this system requires
ecs::Signature signature;
signature.set(ecsManager.getComponentType<NewComponent>());
signature.set(ecsManager.getComponentType<PositionComponent>());
ecsManager.setSystemSignature<NewSystem>(signature);
```

### 🎯 Creating Game Entities

Entities are combinations of components that represent game objects.

**Example: Creating a Power-up Entity**
```cpp
Entity createPowerUp(float x, float y, PowerUpType type) {
  auto& ecsManager = ecs::ECSManager::getInstance();
  
  Entity powerUp = ecsManager.createEntity();
  
  // Add required components
  ecsManager.addComponent(powerUp, ecs::PositionComponent{x, y});
  ecsManager.addComponent(powerUp, ecs::VelocityComponent{0.0f, -50.0f}); // Moving upward
  ecsManager.addComponent(powerUp, ecs::RenderComponent{/* sprite data */});
  ecsManager.addComponent(powerUp, PowerUpComponent{type, 100}); // Custom component
  ecsManager.addComponent(powerUp, ecs::ColliderComponent{20.0f, 20.0f});
  
  return powerUp;
}
```

**Example: Creating an Enemy Entity**
```cpp
Entity createEnemy(float x, float y, EnemyType type) {
  auto& ecsManager = ecs::ECSManager::getInstance();
  
  Entity enemy = ecsManager.createEntity();
  
  ecsManager.addComponent(enemy, ecs::PositionComponent{x, y});
  ecsManager.addComponent(enemy, ecs::VelocityComponent{-100.0f, 0.0f});
  ecsManager.addComponent(enemy, ecs::HealthComponent{100});
  ecsManager.addComponent(enemy, ecs::EnemyComponent{type});
  ecsManager.addComponent(enemy, ecs::RenderComponent{/* sprite data */});
  ecsManager.addComponent(enemy, ecs::ColliderComponent{32.0f, 32.0f});
  ecsManager.addComponent(enemy, ecs::ShootComponent{1.0f}); // Shoot every second
  
  return enemy;
}
```

### 📡 Adding Network Packets

Network packets handle communication between client and server.

**1. Define the packet structure in core/network/Packet.hpp:**
```cpp
// Add to PacketType enum
enum class PacketType : uint8_t {
  // ... existing types ...
  NewPacketType = 0x20, // Use next available ID
};

// Add packet structure
struct ALIGNED NewPacketStruct {
  PacketHeader header;
  uint32_t playerId;
  float someValue;
  uint8_t someFlag;
  // Keep structures aligned and size-efficient
};
```

**2. Create the packet handler:**
```cpp
// server/src/packets/NewPacketHandler.hpp
#pragma once

#include "APacket.hpp"

namespace packet {
  class NewPacketHandler : public APacket {
    public:
      int handlePacket(server::Server& server, server::Client& client,
                      const char* data, std::size_t size) override;
  };
}
```

```cpp
// server/src/packets/NewPacketHandler.cpp
#include "NewPacketHandler.hpp"
#include "Packet.hpp"
#include "Server.hpp"

namespace packet {
  int NewPacketHandler::handlePacket(server::Server& server, server::Client& client,
                                    const char* data, std::size_t size) {
    if (size < sizeof(NewPacketStruct)) {
      return -1; // Invalid packet size
    }
    
    const auto* packet = reinterpret_cast<const NewPacketStruct*>(data);
    
    // Validate packet data
    if (packet->header.type != PacketType::NewPacketType) {
      return -1;
    }
    
    // Process the packet
    // Access: packet->playerId, packet->someValue, etc.
    
    // Example: Update game state
    server.updatePlayerState(client.getId(), packet->someValue);
    
    // Example: Broadcast to other clients if needed
    server.broadcastToOthers(client.getId(), data, size);
    
    return 0; // Success
  }
}
```

**3. Register the packet handler:**
```cpp
// Add to server/src/packets/PacketFactory.hpp
private:
  inline static const std::unordered_map<
      PacketType, std::function<std::unique_ptr<APacket>()>>
      _handlers = {
        // ... existing handlers ...
        {PacketType::NewPacketType,
         []() { return std::make_unique<NewPacketHandler>(); }},
      };
```

### 📋 Best Practices

**Components:**
- Keep components simple data containers (no logic)
- Use descriptive names (e.g., `HealthComponent`, not `HC`)
- Group related data together
- Consider memory layout and alignment

**Systems:**
- One responsibility per system
- Process entities in batches when possible
- Handle edge cases (empty entity sets, missing components)
- Consider system update order dependencies

**Entities:**
- Use factory functions for complex entities
- Document required component combinations
- Clean up entities when no longer needed
- Use entity IDs for cross-system communication

**Packets:**
- Keep packet structures small and efficient
- Validate all input data
- Use proper alignment for network data
- Handle endianness for cross-platform compatibility
- Include packet versioning for protocol evolution

**Performance Tips:**
- Cache component lookups in tight loops
- Use `hasComponent()` before `getComponent()` when uncertain
- Minimize entity creation/destruction during gameplay
- Profile system update performance regularly