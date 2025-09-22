#include "Game.hpp"

game::Game::Game() : _running(false) {
}

game::Game::~Game() {
  stop();
  if (_gameThread.joinable()) {
    _gameThread.join();
  }
}

void game::Game::start() {
  if (_running) {
    return;
  }
  _running = true;
  _gameThread = std::thread(&Game::Game::gameLoop, this);
}

void game::Game::stop() {
  _running = false;
}

void game::Game::gameLoop() {
  while (_running) {
    std::this_thread::sleep_for(std::chrono::milliseconds(16));
  }
}
