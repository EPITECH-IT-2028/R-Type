#pragma once

#include <optional>
#include <unordered_map>
#include "ChatComponent.hpp"
#include "Client.hpp"
#include "ECSManager.hpp"
#include "raylib.h"

namespace ecs {
  class ChatMessagesUI {
    public:
      static ChatMessagesUI init() {
        return ChatMessagesUI();
      }

      void setChatEntity(const std::optional<Entity> &entity) {
        _chatEntity = entity;
      }

      const std::optional<Entity> &getChatEntity() const {
        return _chatEntity;
      }

      void setClient(client::Client *client) {
        _client = client;
      }

      void drawMessagesBox();
      void drawMessages();
      void drawMessageInputField(const ChatComponent &chat);

    private:
      ChatMessagesUI() = default;

      client::Client *_client;
      std::optional<Entity> _chatEntity;
  };

  class MenuUI {
    public:
      static MenuUI init() {
        return MenuUI();
      }

      void setClient(client::Client *client) {
        _client = client;
      }

      bool getShowMenu() const {
        return _showMenu;
      }

      void setShowMenu(bool showMenu) {
        _showMenu = showMenu;
      }

      void drawMenu();

    private:
      MenuUI() : _showMenu(true), _textureLoaded(false) {}

      void loadTexture();
      void drawMenuBackground();
      void drawStartButton();

      client::Client *_client;
      bool _showMenu;
      Texture2D _startScreenTexture;
      bool _textureLoaded;
  };

  class RenderSystem : public System {
    public:
      /**
       * @brief Constructs a RenderSystem bound to the given ECS manager.
       *
       * @param ecsManager Reference to the ECSManager instance that the render
       * system will use; defaults to the global singleton.
       */
      explicit RenderSystem(ECSManager &ecsManager = ECSManager::getInstance())
          : _ecsManager(ecsManager), _client(nullptr) {
      }

      /**
       * @brief Disable copying of RenderSystem instances.
       *
       * The copy constructor is deleted to prevent inadvertent copying of the
       * system, which holds references and non-owning pointers that must remain
       * unique.
       */
      RenderSystem(const RenderSystem &) = delete;
      RenderSystem &operator=(const RenderSystem &) = delete;
      RenderSystem &operator=(RenderSystem &&) = delete;

      ~RenderSystem() noexcept;

      void update(float deltaTime) override;

      void setClient(client::Client *client) {
        _client = client;
        _messagesUI.setClient(client);
        _menuUI.setClient(client);
      }

    private:
      ECSManager &_ecsManager;
      std::unordered_map<std::string, Texture2D> _textureCache;
      client::Client *_client;

      ChatMessagesUI _messagesUI = ChatMessagesUI::init();
      MenuUI _menuUI = MenuUI::init();
  };
}  // namespace ecs
