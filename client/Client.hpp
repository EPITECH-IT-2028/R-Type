#pragma once

#include "ECSManager.hpp"

namespace Client {
  constexpr int OK = 0;
  constexpr int KO = 1;
}  // namespace Client

namespace client {
  class Client {
    public:
      Client() : _ecsManager(ecs::ECSManager::getInstance()) {}
      ~Client() = default;

      void initializeECS();

    private:
      void registerComponent();
      void registerSystem();
      void signSystem();

      void createBackgroundEntities();
      void createPlayerEntity();

      ecs::ECSManager &_ecsManager;
  };
}  // namespace client
