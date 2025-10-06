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

      ecs::ECSManager &_ecsManager;
  };
}  // namespace client
