#pragma once

#include "ECSManager.hpp"
#include "System.hpp"

namespace client {
  class Client;
}

namespace ecs {
  class InputSystem : public System {
    public:
      explicit InputSystem(ECSManager &ecsManager = ECSManager::getInstance())
          : _ecsManager(ecsManager), _client(nullptr), _isChatting(false) {
      }

      void update(float deltaTime) override;

      void setClient(client::Client *client) {
        _client = client;
      }

    private:
      ECSManager &_ecsManager;
      client::Client *_client;
      bool _isChatting;
      std::string _chatMessage;
  };
}  // namespace ecs
