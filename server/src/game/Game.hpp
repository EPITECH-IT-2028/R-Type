#pragma once

#include <thread>

namespace game {

  class Game {
    public:
      Game();
      ~Game();
      void start();
      void stop();

    private:
      void gameLoop();
      std::atomic<bool> _running;
      std::thread _gameThread;
  };

}  // namespace game
