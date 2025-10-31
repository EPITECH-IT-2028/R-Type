#pragma once

#include <chrono>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "Packet.hpp"

/**
 * @brief Convert a PacketType value to a human-readable name.
 *
 * @param type The PacketType value to convert.
 * @return The human-readable name for the packet type; "Unknown(n)" if the
 * value is not recognized, where n is the integer value of the enum.
 */
inline std::string packetTypeToString(PacketType type) {
  switch (type) {
    case PacketType::ChatMessage:
      return "ChatMessage";
    case PacketType::PlayerMove:
      return "Move";
    case PacketType::NewPlayer:
      return "NewPlayer";
    case PacketType::PlayerInfo:
      return "PlayerInfo";
    case PacketType::EnemySpawn:
      return "EnemySpawn";
    case PacketType::EnemyMove:
      return "EnemyMove";
    case PacketType::EnemyDeath:
      return "EnemyDeath";
    case PacketType::PlayerShoot:
      return "PlayerShoot";
    case PacketType::ProjectileSpawn:
      return "ProjectileSpawn";
    case PacketType::ProjectileHit:
      return "ProjectileHit";
    case PacketType::ProjectileDestroy:
      return "ProjectileDestroy";
    case PacketType::GameStart:
      return "GameStart";
    case PacketType::GameEnd:
      return "GameEnd";
    case PacketType::PlayerDisconnected:
      return "PlayerDisconnected";
    case PacketType::Heartbeat:
      return "Heartbeat";
    case PacketType::EnemyHit:
      return "EnemyHit";
    case PacketType::PlayerHit:
      return "PlayerHit";
    case PacketType::PlayerDeath:
      return "PlayerDeath";
    case PacketType::CreateRoom:
      return "CreateRoom";
    case PacketType::JoinRoom:
      return "JoinRoom";
    case PacketType::JoinRoomResponse:
      return "JoinRoomResponse";
    case PacketType::ListRoom:
      return "ListRoom";
    case PacketType::ListRoomResponse:
      return "ListRoomResponse";
    case PacketType::MatchmakingRequest:
      return "MatchmakingRequest";
    case PacketType::MatchmakingResponse:
      return "MatchmakingResponse";
    case PacketType::PlayerInput:
      return "PlayerInput";
    case PacketType::Ack:
      return "Ack";
    case PacketType::RequestChallenge:
      return "RequestChallenge";
    case PacketType::ChallengeResponse:
      return "ChallengeResponse";
    case PacketType::CreateRoomResponse:
      return "CreateRoomResponse";
    default:
      std::stringstream ss;
      ss << "Unknown(" << static_cast<int>(type) << ")";
      return ss.str();
  }
}

/**
 * @brief Determines whether a packet of the given type should be acknowledged.
 *
 * @param type PacketType value to check.
 * @return `true` if packets of this type require an acknowledgement, `false`
 * otherwise.
 */
inline bool shouldAcknowledgePacketType(PacketType type) {
  switch (type) {
    case PacketType::GameStart:
    case PacketType::GameEnd:
    case PacketType::PlayerInfo:
    case PacketType::PlayerShoot:
    case PacketType::PlayerHit:
    case PacketType::PlayerDeath:
    case PacketType::CreateRoom:
    case PacketType::JoinRoom:
    case PacketType::JoinRoomResponse:
    case PacketType::PlayerDisconnected:
    case PacketType::ChatMessage:
    case PacketType::NewPlayer:
    case PacketType::EnemySpawn:
    case PacketType::EnemyDeath:
    case PacketType::EnemyHit:
    case PacketType::ProjectileSpawn:
    case PacketType::ProjectileDestroy:
    case PacketType::MatchmakingRequest:
      return true;
    default:
      return false;
  }
}

struct UnacknowledgedPacket {
    std::shared_ptr<std::vector<uint8_t>> data;
    int resend_count;
    std::chrono::steady_clock::time_point last_sent;
};
