#pragma once

#include <cstdint>
#include "Packet.hpp"

namespace ecs {
  /*
   * ProjectileComponent represents a projectile entity in the game.
   * This component is used to manage the properties and behavior of
   * projectiles.
   */
  struct ProjectileComponent {
      std::uint32_t projectile_id = 0;
      ProjectileType type = ProjectileType::PLAYER_BASIC;
      std::uint32_t owner_id = 0;
      bool is_destroy = false;
      bool is_enemy_projectile = false;
      float speed = 0.0f;
      std::uint32_t sequence_number = 0;
      std::uint32_t damage = 30;
  };

}  // namespace ecs
