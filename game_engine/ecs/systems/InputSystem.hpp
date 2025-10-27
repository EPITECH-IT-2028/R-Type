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
          : _ecsManager(ecsManager), _client(nullptr) {
      }

      void update(float deltaTime) override;

      /**
       * @brief Sets the client used by this input system.
       *
       * Associates the provided client pointer with the InputSystem. Passing `nullptr` disassociates any previously set client.
       *
       * @param client Pointer to the client to associate with the system, or `nullptr` to clear it.
       */
      void setClient(client::Client *client) {
        _client = client;
      }

    private:
      bool loadUIEntities();
      ECSManager &_ecsManager;
      client::Client *_client;
  };
}  // namespace ecs