#pragma once

#include <sstream>
#include <string>
#include "Packet.hpp"

/**
 * @brief Convert a PacketType enum value to its human-readable name.
 *
 * @param type PacketType enum value to convert.
 * @return std::string The corresponding name (e.g., "Message", "Move", "PlayerDeath"); if the value is not recognized, returns "Unknown(n)" where n is the integer value of the enum.
 */
inline std::string packetTypeToString(PacketType type) {
  switch (type) {
    case PacketType::Message:
      return "Message";
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
    case PacketType::PlayerInput:
      return "PlayerInput";
    default:
      std::stringstream ss;
      ss << "Unknown(" << static_cast<int>(type) << ")";
      return ss.str();
  }
}
