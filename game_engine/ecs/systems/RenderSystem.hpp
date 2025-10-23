#pragma once

#include <unordered_map>
#include "Client.hpp"
#include "ECSManager.hpp"
#include "raylib.h"

namespace ecs {
  class RenderSystem : public System {
    public:
      explicit RenderSystem(ECSManager &ecsManager = ECSManager::getInstance())
          : _ecsManager(ecsManager), _client(nullptr) {
      }

      RenderSystem(const RenderSystem &) = delete;
      RenderSystem &operator=(const RenderSystem &) = delete;
      RenderSystem(RenderSystem &&) noexcept = default;
      RenderSystem &operator=(RenderSystem &&) = delete;

      ~RenderSystem() noexcept;

      void update(float deltaTime) override;

      void setClient(client::Client *client) {
        _client = client;
      }

    private:
      ECSManager &_ecsManager;
      std::unordered_map<std::string, Texture2D> _textureCache;
      client::Client *_client;
  };
}  // namespace ecs
