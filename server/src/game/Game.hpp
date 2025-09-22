#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>
#include "ECSManager.hpp"
#include "Player.hpp"

namespace game {

  class Game {
    public:
      Game();
      ~Game();
      void start();
      void stop();

      std::shared_ptr<Player> createPlayer(int player_id,
                                           const std::string &name);

      void destroyPlayer(int player_id);

      std::shared_ptr<Player> getPlayer(int player_id);

      std::vector<std::shared_ptr<Player>> getAllPlayers() const;

    private:
      void gameLoop();
      void initECS();
      std::atomic<bool> _running;
      std::thread _gameThread;

      std::unique_ptr<ecs::ECSManager> _ecsManager;
      std::unordered_map<int, std::shared_ptr<Player>> _players;
      mutable std::mutex _playerMutex;
  };

}  // namespace game
