#pragma once
#include <functional>
#include <memory>
#include <unordered_map>
#include "IPacket.hpp"
#include "Packet.hpp"
#include "PacketHandler.hpp"

namespace packet {
  class PacketHandlerFactory {
    public:
      PacketHandlerFactory() = default;
      ~PacketHandlerFactory() = default;

      std::unique_ptr<IPacket> createHandler(PacketType packetType);

    private:
      inline static const std::unordered_map<
          PacketType, std::function<std::unique_ptr<IPacket>()>>
          _handlers = {{PacketType::Message,
                        []() { return std::make_unique<MessageHandler>(); }},
                        {PacketType::NewPlayer,
                        []() { return std::make_unique<NewPlayerHandler>(); }},
                        {PacketType::PlayerDisconnected,
                        []() { return std::make_unique<PlayerDisconnectedHandler>(); }},
                        {PacketType::Move,
                        []() { return std::make_unique<PlayerMoveHandler>(); }},
                        {PacketType::EnemySpawn,
                        []() { return std::make_unique<EnemySpawnHandler>(); }},
                        {PacketType::EnemyMove,
                        []() { return std::make_unique<EnemyMoveHandler>(); }},
                        {PacketType::EnemyDeath,
                        []() { return std::make_unique<EnemyDeathHandler>(); }}
                      };

  };
}  // namespace packet
