#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>
#include "ECSManager.hpp"
#include "Enemy.hpp"
#include "EnemyComponent.hpp"
#include "EnemySystem.hpp"
#include "Player.hpp"
#include "Projectile.hpp"
#include "Queue.hpp"

namespace game {

  class Game {
    public:
      Game();
      ~Game();
      void start();
      void stop();

      /*  Player Management */
      std::shared_ptr<Player> createPlayer(int player_id,
                                           const std::string &name);

      std::shared_ptr<game::Projectile> createProjectile(
          std::uint32_t projectile_id, std::uint32_t owner_id,
          ProjectileType type, float x, float y);
      void destroyPlayer(int player_id);

      void destroyProjectile(std::uint32_t projectile_id);

      std::shared_ptr<Player> getPlayer(int player_id);

      std::shared_ptr<Projectile> getProjectile(std::uint32_t projectile_id);

      std::vector<std::shared_ptr<Player>> getAllPlayers() const;

      /* Enemy Management */
      std::shared_ptr<Enemy> createEnemy(int enemy_id,
                                         const ecs::EnemyType type);
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

    private:
      void gameLoop();
      void initECS();
      std::atomic<bool> _running;
      std::thread _gameThread;

      void spawnEnemy(float deltaTime);

      std::unique_ptr<ecs::ECSManager> _ecsManager;

      std::shared_ptr<ecs::EnemySystem> _enemySystem;

      std::unordered_map<int, std::shared_ptr<Enemy>> _enemies;

      float _enemySpawnTimer = 0.0f;
      float _enemySpawnInterval = 3.0f;
      int _nextEnemyId = 0;

      std::unordered_map<int, std::shared_ptr<Player>> _players;
      std::unordered_map<std::uint32_t, std::shared_ptr<Projectile>>
          _projectiles;
      mutable std::mutex _ecsMutex;
      mutable std::mutex _playerMutex;
      mutable std::mutex _enemyMutex;
      queue::EventQueue _eventQueue;
      mutable std::mutex _projectileMutex;
  };

}  // namespace game
