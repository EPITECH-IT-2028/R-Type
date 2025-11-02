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
          _handlers = {
              {PacketType::ChatMessage,
               []() { return std::make_unique<ChatMessageHandler>(); }},
              {PacketType::EnemySpawn,
               []() { return std::make_unique<EnemySpawnHandler>(); }},
              {PacketType::ProjectileSpawn,
               []() { return std::make_unique<ProjectileSpawnHandler>(); }},
              {PacketType::ProjectileHit,
               []() { return std::make_unique<ProjectileHitHandler>(); }},
              {PacketType::ProjectileDestroy,
               []() { return std::make_unique<ProjectileDestroyHandler>(); }},
              {PacketType::NewPlayer,
               []() { return std::make_unique<NewPlayerHandler>(); }},
              {PacketType::PlayerDeath,
               []() { return std::make_unique<PlayerDeathHandler>(); }},
              {PacketType::PlayerDisconnected,
               []() { return std::make_unique<PlayerDisconnectedHandler>(); }},
              {PacketType::PlayerMove,
               []() { return std::make_unique<PlayerMoveHandler>(); }},
              {PacketType::EnemyMove,
               []() { return std::make_unique<EnemyMoveHandler>(); }},
              {PacketType::EnemyDeath,
               []() { return std::make_unique<EnemyDeathHandler>(); }},
              {PacketType::GameStart,
               []() { return std::make_unique<GameStartHandler>(); }},
              {PacketType::PlayerShoot,
               []() { return std::make_unique<PlayerShootHandler>(); }},
              {PacketType::Ack,
               []() { return std::make_unique<AckPacketHandler>(); }},
              {PacketType::JoinRoomResponse,
               []() { return std::make_unique<JoinRoomResponseHandler>(); }},
              {PacketType::MatchmakingResponse,
               []() { return std::make_unique<MatchmakingResponseHandler>(); }},
              {PacketType::Pong,
               []() { return std::make_unique<PongHandler>(); }},
              {PacketType::ChallengeResponse,
               []() { return std::make_unique<ChallengeResponseHandler>(); }},
              {PacketType::CreateRoomResponse,
               []() { return std::make_unique<CreateRoomResponseHandler>(); }},
              {PacketType::ScoreboardResponse,
               []() { return std::make_unique<ScoreboardResponseHandler>(); }},
              {PacketType::GameEnd,
               []() { return std::make_unique<GameEndHandler>(); }},
      };
  };
}  // namespace packet
