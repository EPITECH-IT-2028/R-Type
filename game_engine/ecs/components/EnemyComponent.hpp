#pragma once

namespace ecs {

  enum class EnemyType {
    BASIC
  };

  struct EnemyComponent {
      EnemyType type = EnemyType::BASIC;
      bool is_alive = true;
  };
}  // namespace ecs
