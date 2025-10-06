#pragma once

#include "ECSManager.hpp"

namespace client {
  class Client {
    public:
      Client();
      ~Client() = default;

    private:
      void initECS();
      void registerComponent();
      void registerSystem();
      void signSystem();

      void createBackgroundEntities();
      void createPlayerEntity();

      ecs::ECSManager &_ecsManager;
  };
}  // namespace client
