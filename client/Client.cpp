#include "Client.hpp"
#include "BackgroundTagComponent.hpp"
#include "BoundarySystem.hpp"
#include "InputSystem.hpp"
#include "PlayerTagComponent.hpp"
#include "PositionComponent.hpp"
#include "RenderComponent.hpp"
#include "RenderManager.hpp"
#include "ScaleComponent.hpp"
#include "SpeedComponent.hpp"
#include "SpriteComponent.hpp"
#include "VelocityComponent.hpp"
#include "systems/BackgroundSystem.hpp"
#include "systems/MovementSystem.hpp"
#include "systems/RenderSystem.hpp"

namespace client {
  Client::Client(const std::string &host, const std::string &port)
      : _socket(_io_context),
        _host(host),
        _port(port),
        _sequence_number{0},
        _packet_count{0},
        _timeout(TIMEOUT_MS),
        _packetFactory(),
        _ecsManager(ecs::ECSManager::getInstance()) {
    _running.store(false, std::memory_order_release);
  }

  void Client::connect() {
    try {
      asio::ip::udp::resolver resolver(_io_context);
      auto endpoints = resolver.resolve(asio::ip::udp::v4(), _host, _port);
      if (endpoints.empty()) {
        std::cerr << "Failed to resolve host: " << _host << ":" << _port
                  << std::endl;
        return;
      }
      _server_endpoint = *endpoints.begin();
      _socket.open(_server_endpoint.protocol());
      _running.store(true, std::memory_order_release);
      std::cout << "Connected to " << _host << ":" << _port << std::endl;
    } catch (std::exception &e) {
      std::cerr << "Connection error: " << e.what() << std::endl;
    }
  }

  void Client::disconnect() {
    _running.store(false, std::memory_order_release);
    _socket.close();
    std::cout << "Disconnected from server." << std::endl;
  }

  void Client::receivePackets() {
    if (!_running.load(std::memory_order_acquire)) {
      std::cerr << "Client is not connected. Cannot receive packets."
                << std::endl;
      return;
    }

    while (_running.load(std::memory_order_acquire)) {
      try {
        asio::ip::udp::endpoint sender_endpoint;
        std::size_t length = 0;
        asio::error_code ec;

        asio::steady_timer timer(_io_context);
        bool received = false;

        _socket.async_receive_from(
            asio::buffer(_recv_buffer), sender_endpoint,
            [&](const asio::error_code &error, std::size_t bytes_recvd) {
              ec = error;
              length = bytes_recvd;
              received = true;
              timer.cancel();
            });

        timer.expires_after(_timeout);
        timer.async_wait([&](const asio::error_code &error) {
          if (!error && !received) {
            _socket.cancel();
          }
        });

        _io_context.restart();
        _io_context.run();

        if (!received)
          continue;

        if (ec == asio::error::operation_aborted) {
          continue;
        }
        if (ec) {
          std::cerr << "Receive error: " << ec.message() << std::endl;
          continue;
        }

        if (length > 0) {
          const char *data = _recv_buffer.data();
          std::size_t size = length;

          if (size < sizeof(PacketHeader)) {
            std::cerr << "Received packet too small to contain header."
                      << std::endl;
            continue;
          }

          const PacketHeader *header =
              reinterpret_cast<const PacketHeader *>(data);
          PacketType packet_type = static_cast<PacketType>(header->type);

          auto handler = _packetFactory.createHandler(packet_type);
          if (handler) {
            int result = handler->handlePacket(*this, data, size);
            if (result != 0) {
              std::cerr << "Error handling packet of type "
                        << static_cast<int>(packet_type) << ": " << result
                        << std::endl;
            }
          } else {
            std::cerr << "No handler for packet type "
                      << static_cast<int>(packet_type) << std::endl;
          }
        }
      } catch (std::exception &e) {
        std::cerr << "Receive error: " << e.what() << std::endl;
      }
    }
  }

  void Client::initializeECS() {
    registerComponent();
    registerSystem();
    signSystem();
    createBackgroundEntities();
    createPlayerEntity();
  }

  void Client::registerComponent() {
    _ecsManager.registerComponent<ecs::PositionComponent>();
    _ecsManager.registerComponent<ecs::VelocityComponent>();
    _ecsManager.registerComponent<ecs::RenderComponent>();
    _ecsManager.registerComponent<ecs::SpeedComponent>();
    _ecsManager.registerComponent<ecs::SpriteComponent>();
    _ecsManager.registerComponent<ecs::ScaleComponent>();
    _ecsManager.registerComponent<ecs::BackgroundTagComponent>();
    _ecsManager.registerComponent<ecs::PlayerTagComponent>();
  }

  void Client::registerSystem() {
    _ecsManager.registerSystem<ecs::BackgroundSystem>();
    _ecsManager.registerSystem<ecs::MovementSystem>();
    _ecsManager.registerSystem<ecs::RenderSystem>();
    _ecsManager.registerSystem<ecs::InputSystem>();
    _ecsManager.registerSystem<ecs::BoundarySystem>();
  }

  void Client::signSystem() {
    {
      Signature signature;
      signature.set(_ecsManager.getComponentType<ecs::PositionComponent>());
      signature.set(_ecsManager.getComponentType<ecs::RenderComponent>());
      signature.set(_ecsManager.getComponentType<ecs::BackgroundTagComponent>());
      _ecsManager.setSystemSignature<ecs::BackgroundSystem>(signature);
    }
    {
      Signature signature;
      signature.set(_ecsManager.getComponentType<ecs::PositionComponent>());
      signature.set(_ecsManager.getComponentType<ecs::VelocityComponent>());
      _ecsManager.setSystemSignature<ecs::MovementSystem>(signature);
    }
    {
      Signature signature;
      signature.set(_ecsManager.getComponentType<ecs::PositionComponent>());
      signature.set(_ecsManager.getComponentType<ecs::RenderComponent>());
      _ecsManager.setSystemSignature<ecs::RenderSystem>(signature);
    }
    {
      Signature signature;
      signature.set(_ecsManager.getComponentType<ecs::VelocityComponent>());
      signature.set(_ecsManager.getComponentType<ecs::SpeedComponent>());
      signature.set(_ecsManager.getComponentType<ecs::PlayerTagComponent>());
      _ecsManager.setSystemSignature<ecs::InputSystem>(signature);
    }
    {
      Signature signature;
      signature.set(_ecsManager.getComponentType<ecs::PositionComponent>());
      signature.set(_ecsManager.getComponentType<ecs::SpriteComponent>());
      signature.set(_ecsManager.getComponentType<ecs::PlayerTagComponent>());
      _ecsManager.setSystemSignature<ecs::BoundarySystem>(signature);
    }
  }

  void Client::createBackgroundEntities() {
    Image backgroundImage = LoadImage(renderManager::BG_PATH);
    float screenHeight = GetScreenHeight();
    float aspectRatio = 1.0f;
    float scaledWidth = screenHeight;
    if (backgroundImage.data != nullptr && backgroundImage.height > 0) {
      aspectRatio = static_cast<float>(backgroundImage.width) /
                    static_cast<float>(backgroundImage.height);
      scaledWidth = screenHeight * aspectRatio;
      UnloadImage(backgroundImage);
    }

    auto background1 = _ecsManager.createEntity();
    _ecsManager.addComponent<ecs::PositionComponent>(background1, {0.0f, 0.0f});
    _ecsManager.addComponent<ecs::VelocityComponent>(
        background1, {-renderManager::SCROLL_SPEED, 0.0f});
    _ecsManager.addComponent<ecs::RenderComponent>(
        background1, {renderManager::BG_PATH});
    _ecsManager.addComponent<ecs::BackgroundTagComponent>(background1, {});

    auto background2 = _ecsManager.createEntity();
    _ecsManager.addComponent<ecs::PositionComponent>(background2, {scaledWidth, 0.0f});
    _ecsManager.addComponent<ecs::VelocityComponent>(
        background2, {-renderManager::SCROLL_SPEED, 0.0f});
    _ecsManager.addComponent<ecs::RenderComponent>(
        background2, {renderManager::BG_PATH});
    _ecsManager.addComponent<ecs::BackgroundTagComponent>(background2, {});
  }

  void Client::createPlayerEntity() {
    auto player = _ecsManager.createEntity();
    _ecsManager.addComponent<ecs::PositionComponent>(player, {100.0f, 100.0f});
    _ecsManager.addComponent<ecs::VelocityComponent>(player, {0.0f, 0.0f});
    _ecsManager.addComponent<ecs::SpeedComponent>(player, {PLAYER_SPEED});
    _ecsManager.addComponent<ecs::RenderComponent>(
        player, {renderManager::PLAYER_PATH});
    _ecsManager.addComponent<ecs::SpriteComponent>(player, {0.0f, 0.0f, 33.0f, 17.0f});
    _ecsManager.addComponent<ecs::ScaleComponent>(player, {2.0f, 2.0f});
    _ecsManager.addComponent<ecs::PlayerTagComponent>(player, {});
  }
}  // namespace client
