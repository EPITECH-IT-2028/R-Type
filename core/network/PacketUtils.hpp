#pragma once

#include <sstream>
#include <string>
#include "Packet.hpp"

inline std::string packetTypeToString(PacketType type) {
  switch (type) {
    case PacketType::Message:
      return "Message";
    case PacketType::Move:
      return "Move";
    case PacketType::NewPlayer:
      return "NewPlayer";
    case PacketType::PlayerInfo:
      return "PlayerInfo";
    case PacketType::Position:
      return "Position";
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
    case PacketType::InputPlayer:
      return "InputPlayer";
    case PacketType::PositionPlayer:
      return "PositionPlayer";
    default:
      std::stringstream ss;
      ss << "Unknown(" << static_cast<int>(type) << ")";
      return ss.str();
  }
}
