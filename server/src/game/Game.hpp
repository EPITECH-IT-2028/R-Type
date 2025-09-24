#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>
#include "ECSManager.hpp"
#include "Player.hpp"
#include "Projectile.hpp"

namespace game {

  class Game {
    public:
      Game();
      ~Game();
      void start();
      void stop();

      std::shared_ptr<Player> createPlayer(int player_id,
                                           const std::string &name);

      std::shared_ptr<game::Projectile> createProjectile(
          int projectile_id, uint32_t entity_id, const ProjectileType &type,
          float x, float y);

      void destroyPlayer(int player_id);

      void destroyProjectile(int projectile_id);

      std::shared_ptr<Player> getPlayer(int player_id);

      std::shared_ptr<Projectile> getProjectile(int projectile_id);

      std::vector<std::shared_ptr<Player>> getAllPlayers() const;

      std::vector<std::shared_ptr<Projectile>> getAllProjectiles() const;

    private:
      void gameLoop();
      void initECS();
      std::atomic<bool> _running;
      std::thread _gameThread;

      std::unique_ptr<ecs::ECSManager> _ecsManager;
      std::unordered_map<int, std::shared_ptr<Player>> _players;
      std::unordered_map<int, std::shared_ptr<Projectile>> _projectiles;
      mutable std::mutex _playerMutex;
  };

}  // namespace game
