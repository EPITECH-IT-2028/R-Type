#pragma once

#include <optional>
#include <unordered_map>
#include "ChatComponent.hpp"
#include "Client.hpp"
#include "ECSManager.hpp"
#include "raylib.h"

namespace ecs {
  class RenderSystem : public System {
    public:
      /**
       * @brief Constructs a RenderSystem bound to the given ECS manager.
       *
       * @param ecsManager Reference to the ECSManager instance that the render system will use; defaults to the global singleton.
       */
      explicit RenderSystem(ECSManager &ecsManager = ECSManager::getInstance())
          : _ecsManager(ecsManager), _client(nullptr) {
      }

      /**
 * @brief Disable copying of RenderSystem instances.
 *
 * The copy constructor is deleted to prevent inadvertent copying of the system,
 * which holds references and non-owning pointers that must remain unique.
 */
RenderSystem(const RenderSystem &) = delete;
      RenderSystem &operator=(const RenderSystem &) = delete;
      RenderSystem(RenderSystem &&) noexcept = default;
      RenderSystem &operator=(RenderSystem &&) = delete;

      ~RenderSystem() noexcept;

      void update(float deltaTime) override;

      /**
       * @brief Associate a client instance with the render system.
       *
       * Attaches the provided client so the render system can access client-side data during rendering.
       * Passing `nullptr` clears the stored client.
       *
       * @param client Pointer to the client to set, or `nullptr` to unset the current client.
       */
      void setClient(client::Client *client) {
        _client = client;
      }

    private:
      void drawMessagesBox();
      void drawMessages();
      void drawMessageInputField(const ChatComponent &chat);

      ECSManager &_ecsManager;
      std::unordered_map<std::string, Texture2D> _textureCache;
      client::Client *_client;
      std::optional<Entity> _chatEntity;
  };
}  // namespace ecs