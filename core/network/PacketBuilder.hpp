#pragma once

#include <cstdint>
#include <cstring>
#include <ctime>
#include <string>
#include "Packet.hpp"

struct PacketBuilder {
    static MessagePacket makeMessage(const std::string &msg, std::uint32_t player_id) {
      MessagePacket packet{};
      packet.header.type = PacketType::Message;
      packet.header.size = sizeof(packet);
      packet.timestamp = static_cast<uint32_t>(time(nullptr));
      strncpy(packet.message, msg.c_str(), sizeof(packet.message) - 1);
      packet.message[sizeof(packet.message) - 1] = '\0';
      packet.player_id = player_id;
      return packet;
    }

    static NewPlayerPacket makeNewPlayer(uint32_t player_id, float x, float y,
                                         float speed,
                                         uint32_t max_health = 100) {
      NewPlayerPacket packet{};
      packet.header.type = PacketType::NewPlayer;
      packet.header.size = sizeof(packet);
      packet.player_id = player_id;
      packet.x = x;
      packet.y = y;
      packet.speed = speed;
      packet.max_health = max_health;
      return packet;
    }

    static PositionPacket makePosition(float x, float y, uint32_t seq) {
      PositionPacket packet{};
      packet.header.type = PacketType::Position;
      packet.header.size = sizeof(packet);
      packet.x = x;
      packet.y = y;
      packet.sequence_number = seq;
      return packet;
    }

    static MovePacket makeMove(uint32_t player_id, uint32_t seq, float x,
                               float y) {
      MovePacket packet{};
      packet.header.type = PacketType::Move;
      packet.header.size = sizeof(packet);
      packet.player_id = player_id;
      packet.sequence_number = seq;
      packet.x = x;
      packet.y = y;
      return packet;
    }

    /**
     * @brief Create a PlayerInfo packet containing the player's name.
     *
     * @param name Player's name; copied into the packet's name field and
     * truncated if it exceeds the field size. The stored name is guaranteed to
     * be null-terminated.
     * @return PlayerInfoPacket Packet with header.type set to
     * PacketType::PlayerInfo, header.size populated, and the name field filled
     * from `name`.
     */
    static PlayerInfoPacket makePlayerInfo(const std::string &name) {
      PlayerInfoPacket packet{};
      packet.header.type = PacketType::PlayerInfo;
      packet.header.size = sizeof(packet);
      strncpy(packet.name, name.c_str(), sizeof(packet.name) - 1);
      packet.name[sizeof(packet.name) - 1] = '\0';
      return packet;
    }

    /**
     * @brief Create a PlayerHitPacket describing a hit applied to a player.
     *
     * @param player_id ID of the player who was hit.
     * @param damage Amount of damage applied to the player.
     * @param x X coordinate of the hit location.
     * @param y Y coordinate of the hit location.
     * @param sequence_number Sequence number associated with this event.
     * @return PlayerHitPacket Packet populated with the provided player ID,
     * damage, hit coordinates, and sequence number.
     */
    static PlayerHitPacket makePlayerHit(std::uint32_t player_id,
                                         std::uint32_t damage, float x, float y,
                                         int sequence_number) {
      PlayerHitPacket packet{};
      packet.header.type = PacketType::PlayerHit;
      packet.header.size = sizeof(packet);
      packet.player_id = player_id;
      packet.damage = damage;
      packet.x = x;
      packet.y = y;
      packet.sequence_number = sequence_number;
      return packet;
    }

    /**
     * @brief Constructs an EnemySpawnPacket with the enemy's identity, type,
     * position, velocity, and health.
     *
     * @param enemy_id Unique identifier for the enemy.
     * @param type Enemy type.
     * @param x Spawn X coordinate.
     * @param y Spawn Y coordinate.
     * @param vx Initial velocity in the X direction.
     * @param vy Initial velocity in the Y direction.
     * @param health Current health.
     * @param max_health Maximum health.
     * @return EnemySpawnPacket Packet with fields initialized and its header
     * indicating an EnemySpawn.
     */
    static EnemySpawnPacket makeEnemySpawn(uint32_t enemy_id, EnemyType type,
                                           float x, float y, float vx, float vy,
                                           uint32_t health,
                                           uint32_t max_health) {
      EnemySpawnPacket packet{};
      packet.header.type = PacketType::EnemySpawn;
      packet.header.size = sizeof(packet);
      packet.enemy_id = enemy_id;
      packet.enemy_type = type;
      packet.x = x;
      packet.y = y;
      packet.velocity_x = vx;
      packet.velocity_y = vy;
      packet.health = health;
      packet.max_health = max_health;
      return packet;
    }

    static EnemyMovePacket makeEnemyMove(uint32_t enemy_id, float x, float y,
                                         float velocity_x, float velocity_y,
                                         int seq) {
      EnemyMovePacket packet{};
      packet.header.type = PacketType::EnemyMove;
      packet.header.size = sizeof(packet);
      packet.enemy_id = enemy_id;
      packet.x = x;
      packet.y = y;
      packet.velocity_x = velocity_x;
      packet.velocity_y = velocity_y;
      packet.sequence_number = seq;
      return packet;
    }

    /**
     * @brief Create an EnemyDeath packet describing an enemy's death and
     * awarding score to a player.
     *
     * @param enemy_id Unique identifier of the enemy.
     * @param death_x X coordinate of the enemy's death location.
     * @param death_y Y coordinate of the enemy's death location.
     * @param player_id ID of the player credited with the kill.
     * @param score Score awarded to the player for the kill.
     * @return EnemyDeathPacket Populated packet containing enemy_id, death
     * coordinates, player_id, and score; packet header is set to
     * `PacketType::EnemyDeath` and `header.size` is set to the packet's size.
     */
    static EnemyDeathPacket makeEnemyDeath(uint32_t enemy_id, float death_x,
                                           float death_y,
                                           std::uint32_t player_id,
                                           std::uint32_t score) {
      EnemyDeathPacket packet{};
      packet.header.type = PacketType::EnemyDeath;
      packet.header.size = sizeof(packet);
      packet.enemy_id = enemy_id;
      packet.death_x = death_x;
      packet.death_y = death_y;
      packet.player_id = player_id;
      packet.score = score;
      return packet;
    }

    /**
     * @brief Constructs an EnemyHit packet describing damage dealt to an enemy.
     *
     * @param enemy_id Identifier of the enemy that was hit.
     * @param hit_x World X coordinate where the hit occurred.
     * @param hit_y World Y coordinate where the hit occurred.
     * @param damage Amount of damage applied to the enemy.
     * @param sequence_number Sequence number for ordering the event.
     * @return EnemyHitPacket Packet populated with header, enemy id, hit
     * position, damage, and sequence number.
     */
    static EnemyHitPacket makeEnemyHit(uint32_t enemy_id, float hit_x,
                                       float hit_y, float damage,
                                       int sequence_number) {
      EnemyHitPacket packet{};
      packet.header.type = PacketType::EnemyHit;
      packet.header.size = sizeof(packet);
      packet.enemy_id = enemy_id;
      packet.hit_x = hit_x;
      packet.hit_y = hit_y;
      packet.damage = damage;
      packet.sequence_number = sequence_number;
      return packet;
    }

    /**
     * @brief Create a PlayerShoot packet containing shot position, projectile
     * type, and sequence number.
     *
     * @param x X coordinate of the shot.
     * @param y Y coordinate of the shot.
     * @param projectile_type Type of the projectile fired.
     * @param seq Sequence number used for ordering the shot.
     * @return PlayerShootPacket Packet with position, projectile type, and
     * sequence number populated.
     */
    static PlayerShootPacket makePlayerShoot(float x, float y,
                                             ProjectileType projectile_type,
                                             uint32_t seq) {
      PlayerShootPacket packet{};
      packet.header.type = PacketType::PlayerShoot;
      packet.header.size = sizeof(packet);
      packet.x = x;
      packet.y = y;
      packet.projectile_type = projectile_type;
      packet.sequence_number = seq;
      return packet;
    }

    static ProjectileSpawnPacket makeProjectileSpawn(
        uint32_t projectile_id, ProjectileType type, float x, float y,
        float vel_x, float vel_y, bool is_enemy, uint32_t damage,
        uint32_t owner_id) {
      ProjectileSpawnPacket packet{};
      packet.header.type = PacketType::ProjectileSpawn;
      packet.header.size = sizeof(packet);
      packet.projectile_id = projectile_id;
      packet.projectile_type = type;
      packet.x = x;
      packet.y = y;
      packet.velocity_x = vel_x;
      packet.velocity_y = vel_y;
      packet.is_enemy_projectile = is_enemy;
      packet.damage = damage;
      packet.owner_id = owner_id;
      return packet;
    }

    static ProjectileHitPacket makeProjectileHit(uint32_t projectile_id,
                                                 uint32_t target_id,
                                                 float hit_x, float hit_y,
                                                 uint8_t target_is_player) {
      ProjectileHitPacket packet{};
      packet.header.type = PacketType::ProjectileHit;
      packet.header.size = sizeof(packet);
      packet.projectile_id = projectile_id;
      packet.target_id = target_id;
      packet.hit_x = hit_x;
      packet.hit_y = hit_y;
      packet.target_is_player = target_is_player;
      return packet;
    }

    static ProjectileDestroyPacket makeProjectileDestroy(uint32_t projectile_id,
                                                         float x, float y) {
      ProjectileDestroyPacket packet{};
      packet.header.type = PacketType::ProjectileDestroy;
      packet.header.size = sizeof(packet);
      packet.projectile_id = projectile_id;
      packet.x = x;
      packet.y = y;
      return packet;
    }

    static GameStartPacket makeGameStart(bool started) {
      GameStartPacket packet{};
      packet.header.type = PacketType::GameStart;
      packet.header.size = sizeof(packet);
      packet.game_start = started;
      return packet;
    }

    /**
     * @brief Constructs a GameEnd packet indicating whether the game has ended.
     *
     * @param ended `true` if the game has ended, `false` if the game is
     * ongoing.
     * @return GameEndPacket Packet with header fields populated and `game_end`
     * set to `ended`.
     */
    static GameEndPacket makeGameEnd(bool ended) {
      GameEndPacket packet{};
      packet.header.type = PacketType::GameEnd;
      packet.header.size = sizeof(packet);
      packet.game_end = ended;
      return packet;
    }

    /**
     * @brief Constructs a PlayerDeath packet describing a player's death
     * location.
     *
     * Builds a PlayerDeathPacket with the header type and size set, and fills
     * the player identifier and world coordinates where the player died.
     *
     * @param player_id Identifier of the player who died.
     * @param x World X coordinate of the death location.
     * @param y World Y coordinate of the death location.
     * @return PlayerDeathPacket Populated packet ready for transmission.
     */
    static PlayerDeathPacket makePlayerDeath(uint32_t player_id, float x,
                                             float y) {
      PlayerDeathPacket packet{};
      packet.header.type = PacketType::PlayerDeath;
      packet.header.size = sizeof(packet);
      packet.player_id = player_id;
      packet.x = x;
      packet.y = y;
      return packet;
    }

    /**
     * @brief Construct a PlayerDisconnect packet for the given player.
     *
     * @param player_id ID of the player that disconnected.
     * @return PlayerDisconnectPacket Packet with header type set to
     * PacketType::PlayerDisconnected, header size set to the packet's sizeof,
     * and player_id populated.
     */
    static PlayerDisconnectPacket makePlayerDisconnect(uint32_t player_id) {
      PlayerDisconnectPacket packet{};
      packet.header.type = PacketType::PlayerDisconnected;
      packet.header.size = sizeof(packet);
      packet.player_id = player_id;
      return packet;
    }

    /**
     * @brief Construct a heartbeat packet for the given player.
     *
     * @param player_id Player identifier to embed in the packet.
     * @return HeartbeatPlayerPacket Packet whose header.type is
     * PacketType::Heartbeat, header.size is set to the packet size, and
     * player_id is set to the provided value.
     */
    static HeartbeatPlayerPacket makeHeartbeatPlayer(uint32_t player_id) {
      HeartbeatPlayerPacket packet{};
      packet.header.type = PacketType::Heartbeat;
      packet.header.size = sizeof(packet);
      packet.player_id = player_id;
      return packet;
    }

    static InputPlayerPacket makeInputPlayer(MovementInputType input, int sequence_number) {
      InputPlayerPacket packet{};
      packet.header.type = PacketType::InputPlayer;
      packet.header.size = sizeof(packet);
      packet.input = input;
      packet.sequence_number = sequence_number;
      return packet;
    }

    static PositionPlayerPacket makePositionPlayer(uint32_t player_id, float x, float y, std::uint32_t sequence_number) {
      PositionPlayerPacket packet{};
      packet.header.type = PacketType::PositionPlayer;
      packet.header.size = sizeof(packet);
      packet.player_id = player_id;
      packet.x = x;
      packet.y = y;
      packet.sequence_number = 0;
      return packet;
    }
};
