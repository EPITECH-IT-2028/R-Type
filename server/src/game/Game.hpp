#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>
#include "CollisionSystem.hpp"
#include "ECSManager.hpp"
#include "Enemy.hpp"
#include "EnemySystem.hpp"
#include "Player.hpp"
#include "Projectile.hpp"
#include "ProjectileSystem.hpp"
#include "Queue.hpp"

namespace game {

  class Game {
    public:
      Game();
      ~Game();
      void start();
      void stop();

      /*  Player Management */
      std::shared_ptr<Player> createPlayer(std::uint32_t player_id,
                                           const std::string &name);

      std::shared_ptr<game::Projectile> createProjectile(
          std::uint32_t projectile_id, std::uint32_t owner_id,
          ProjectileType type, float x, float y, float vx, float vy);
      void destroyPlayer(int player_id);

      void destroyProjectile(std::uint32_t projectile_id);

      std::shared_ptr<Player> getPlayer(int player_id);

      std::shared_ptr<Projectile> getProjectile(std::uint32_t projectile_id);

      std::vector<std::shared_ptr<Player>> getAllPlayers() const;

      /* Enemy Management */
      std::shared_ptr<Enemy> createEnemy(int enemy_id, const EnemyType type);
      void destroyEnemy(int enemy_id);
      std::shared_ptr<Enemy> getEnemy(int enemy_id);
      std::vector<std::shared_ptr<Enemy>> getAllEnemies() const;

      ecs::ECSManager &getECSManager() {
        return *_ecsManager;
      }

      std::shared_ptr<ecs::EnemySystem> getEnemySystem() {
        return _enemySystem;
      }

      queue::EventQueue &getEventQueue() {
        return _eventQueue;
      }

      std::vector<std::shared_ptr<Projectile>> getAllProjectiles() const;

      std::uint32_t getNextProjectileId() noexcept {
        return _nextProjectileId++;
      }

      /**
       * @brief Clears all entities from the game, including players, enemies,
       * and projectiles. This method locks the ECS manager to safely destroy
       * all entities and resets internal state.
       *
       * After calling this method, the game will have no entities and internal
       * ID counters for enemies and projectiles will be reset.
       *
       * @note This does not stop the game loop; it only clears entities.
       *
       */
      void clearAllEntities();

      /**
       * @brief Retrieves the current delta time used for game updates.
       *
       * @return float The delta time in seconds between the current and
       * previous update.
       */
      float getDeltaTime() const {
        return _deltaTime.load();
      }

      /**
       * @brief Set the game's sequence number used for ordering events and
       * updates.
       *
       * @param value New sequence number to assign.
       */
      void setSequenceNumber(std::uint32_t value) {
        _sequence_number = value;
      }
      /**
       * @brief Atomically increments the internal sequence number by one and
       * return the new value.
       *
       * Advances the game's sequence number used for event and update ordering.
       */
      std::uint32_t fetchAndIncrementSequenceNumber() {
        return _sequence_number.fetch_add(1, std::memory_order_relaxed);
      }

      void incrementSequenceNumber() {
        _sequence_number.fetch_add(1, std::memory_order_relaxed);
      }

      std::uint32_t getSequenceNumber() {
        return _sequence_number.load(std::memory_order_acquire);
      }

    private:
      void gameLoop();
      void initECS();
      std::atomic<bool> _running;
      std::thread _gameThread;
      std::atomic<float> _deltaTime{0.0f};

      void spawnEnemy(float deltaTime);

      std::shared_ptr<ecs::EnemySystem> _enemySystem;
      std::shared_ptr<ecs::ProjectileSystem> _projectileSystem;
      std::shared_ptr<ecs::CollisionSystem> _collisionSystem;

      std::unordered_map<int, std::shared_ptr<Enemy>> _enemies;
      std::unordered_map<int, std::shared_ptr<Player>> _players;
      std::unordered_map<std::uint32_t, std::shared_ptr<Projectile>>
          _projectiles;

      std::atomic<std::uint32_t> _sequence_number{0};
      float _enemySpawnTimer = 0.0f;
      float _enemySpawnInterval = 5.0f;
      int _nextEnemyId = 0;
      std::atomic<std::uint32_t> _nextProjectileId{0};

      std::unique_ptr<ecs::ECSManager> _ecsManager;
      mutable std::mutex _ecsMutex;
      mutable std::mutex _playerMutex;
      mutable std::mutex _enemyMutex;
      queue::EventQueue _eventQueue;
      mutable std::mutex _projectileMutex;
  };

}  // namespace game
