#pragma once
#include <functional>
#include <memory>
#include <unordered_map>
#include "APacket.hpp"
#include "Packet.hpp"
#include "PacketHandler.hpp"

namespace packet {

  class PacketHandlerFactory {
    public:
      PacketHandlerFactory() = default;
      ~PacketHandlerFactory() = default;

      std::unique_ptr<APacket> createHandler(PacketType packetType);

    private:
      inline static const std::unordered_map<
          PacketType, std::function<std::unique_ptr<APacket>()>>
          _handlers = {
              {PacketType::ChatMessage,
               []() { return std::make_unique<ChatMessageHandler>(); }},
              {PacketType::PlayerInfo,
               []() { return std::make_unique<PlayerInfoHandler>(); }},
              {PacketType::Heartbeat,
               []() { return std::make_unique<HeartbeatPlayerHandler>(); }},
              {PacketType::PlayerDisconnected,
               []() { return std::make_unique<PlayerDisconnectedHandler>(); }},
              {PacketType::PlayerShoot,
               []() { return std::make_unique<PlayerShootHandler>(); }},
              {PacketType::CreateRoom,
               []() { return std::make_unique<CreateRoomHandler>(); }},
              {PacketType::JoinRoom,
               []() { return std::make_unique<JoinRoomHandler>(); }},
              {PacketType::LeaveRoom,
               []() { return std::make_unique<LeaveRoomHandler>(); }},
              {PacketType::ListRoom,
               []() { return std::make_unique<ListRoomHandler>(); }},
              {PacketType::MatchmakingRequest,
               []() { return std::make_unique<MatchmakingRequestHandler>(); }},
              {PacketType::PlayerInput,
               []() { return std::make_unique<PlayerInputHandler>(); }},
              {PacketType::Ack,
               []() { return std::make_unique<AckPacketHandler>(); }},
              {PacketType::RequestChallenge,
               []() { return std::make_unique<RequestChallengeHandler>(); }},
              {PacketType::ScoreboardRequest,
               []() { return std::make_unique<ScoreboardRequestHandler>(); }},
      };
  };

}  // namespace packet
