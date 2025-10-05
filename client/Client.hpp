#pragma once

#include "ECSManager.hpp"

namespace client {
  class Client {
    public:
      Client();
      ~Client();

    private:
      void initECS();

      ecs::ECSManager &_ecsManager;
      bool _running;
  };
}  // namespace client
