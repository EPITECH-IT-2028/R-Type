#pragma once

#include "ECSManager.hpp"

namespace client {
  class Client {
    public:
      Client();
      ~Client() = default;

    private:
      void initECS();

      ecs::ECSManager &_ecsManager;
  };
}  // namespace client
