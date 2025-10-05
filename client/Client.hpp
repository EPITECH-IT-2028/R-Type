#pragma once

#include <memory>
#include "ECSManager.hpp"

namespace client {
  class Client {
    public:
      Client();
      ~Client();

    private:
      ecs::ECSManager &_ecsManager;
      void initECS();
      bool _running;
  };
}  // namespace client
