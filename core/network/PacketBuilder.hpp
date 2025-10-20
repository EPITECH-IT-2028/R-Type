#pragma once

#include <cstdint>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>
#include "Macro.hpp"
#include "Packet.hpp"

struct PacketBuilder {
    static MessagePacket makeMessage(const std::string &msg,
                                     std::uint32_t player_id) {
      MessagePacket packet{};
      packet.header.type = PacketType::Message;
      packet.header.size = sizeof(packet);
      packet.timestamp = static_cast<uint32_t>(time(nullptr));
      strncpy(packet.message, msg.c_str(), sizeof(packet.message) - 1);
      packet.message[sizeof(packet.message) - 1] = '\0';
      packet.player_id = player_id;
      return packet;
    }

    /**
     * @brief Creates a NewPlayerPacket for a newly joined player.
     *
     * The returned packet contains the player's identity, position, movement
     * speed, and maximum health, and has its packet header initialized for a
     * NewPlayer.
     *
     * @param player_id Unique identifier for the player.
     * @param x Initial X position of the player.
     * @param y Initial Y position of the player.
     * @param speed Initial movement speed of the player.
     * @param max_health Maximum health for the player (default: 100).
     * @return NewPlayerPacket Populated packet ready to be serialized and sent.
     */
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

    /**
     * @brief Create a movement update for a player.
     *
     * @param player_id ID of the player who moved.
     * @param seq Sequence number used to order this movement update.
     * @param x New X coordinate of the player.
     * @param y New Y coordinate of the player.
     * @return PlayerMovePacket Packet populated with header.type set to
     * PlayerMove, header.size set to the packet size, and fields: player_id,
     * sequence_number, x, and y.
     */
    static PlayerMovePacket makePlayerMove(uint32_t player_id, uint32_t seq,
                                           float x, float y) {
      PlayerMovePacket packet{};
      packet.header.type = PacketType::PlayerMove;
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
     * @brief Create a HeartbeatPlayerPacket for the specified player.
     *
     * @param player_id Player identifier to embed in the packet.
     * @return HeartbeatPlayerPacket with header.type set to
     * PacketType::Heartbeat, header.size set to the packet size, and player_id
     * set to the provided value.
     */
    static HeartbeatPlayerPacket makeHeartbeatPlayer(uint32_t player_id) {
      HeartbeatPlayerPacket packet{};
      packet.header.type = PacketType::Heartbeat;
      packet.header.size = sizeof(packet);
      packet.player_id = player_id;
      return packet;
    }

    static CreateRoomPacket makeCreateRoom(const std::string &room_name,
                                           uint32_t max_players,
                                           const std::string &password = "") {
      CreateRoomPacket packet{};
      packet.header.type = PacketType::CreateRoom;
      packet.header.size = sizeof(packet);
      strncpy(packet.room_name, room_name.c_str(),
              sizeof(packet.room_name) - 1);
      packet.room_name[sizeof(packet.room_name) - 1] = '\0';
      packet.max_players = max_players;
      packet.is_private = !password.empty();
      if (packet.is_private) {
        strncpy(packet.password, password.c_str(), sizeof(packet.password) - 1);
        packet.password[sizeof(packet.password) - 1] = '\0';
      } else {
        std::memset(packet.password, 0, sizeof(packet.password));
      }
      return packet;
    }

    static JoinRoomPacket makeJoinRoom(uint32_t room_id,
                                       const std::string &password) {
      JoinRoomPacket packet{};
      packet.header.type = PacketType::JoinRoom;
      packet.header.size = sizeof(packet);
      packet.room_id = room_id;
      strncpy(packet.password, password.c_str(), sizeof(packet.password) - 1);
      packet.password[sizeof(packet.password) - 1] = '\0';
      return packet;
    }

    static JoinRoomResponsePacket makeJoinRoomResponse(
        const RoomError &error_code) {
      JoinRoomResponsePacket packet{};
      packet.header.type = PacketType::JoinRoomResponse;
      packet.header.size = sizeof(packet);
      packet.error_code = error_code;
      return packet;
    }

    static LeaveRoomPacket makeLeaveRoom(uint32_t room_id) {
      LeaveRoomPacket packet{};
      packet.header.type = PacketType::LeaveRoom;
      packet.header.size = sizeof(packet);
      packet.room_id = room_id;
      return packet;
    }

    static ListRoomPacket makeListRoom() {
      ListRoomPacket packet{};
      packet.header.type = PacketType::ListRoom;
      packet.header.size = sizeof(packet);
      return packet;
    }

    /**
     * @brief Create a ListRoomResponsePacket containing room information.
     * @param room_count Number of rooms included in the packet.
     * @param rooms Pointer to an array of RoomInfo structures describing
     * each room.
     * @return ListRoomResponsePacket Packet with header type set to
     * PacketType::ListRoom, header size set, room_count populated,
     * and room information copied from the provided array.
     */
    static ListRoomResponsePacket makeListRoomResponse(
        const std::vector<RoomInfo> &rooms) {
      ListRoomResponsePacket packet{};
      packet.header.type = PacketType::ListRoomResponse;
      packet.header.size = sizeof(packet);
      packet.room_count =
          static_cast<uint32_t>(std::min<std::size_t>(rooms.size(), MAX_ROOMS));

      for (std::size_t i = 0; i < packet.room_count; ++i) {
        packet.rooms[i] = rooms[i];
      }
      return packet;
    }

    static MatchmakingRequestPacket makeMatchmakingRequest() {
      MatchmakingRequestPacket packet{};
      packet.header.type = PacketType::MatchmakingRequest;
      packet.header.size = sizeof(packet);
      return packet;
    }

    static MatchmakingResponsePacket makeMatchmakingResponse(
        const RoomError &error_code) {
      MatchmakingResponsePacket packet{};
      packet.header.type = PacketType::MatchmakingResponse;
      packet.header.size = sizeof(packet);
      packet.error_code = error_code;
      return packet;
    };

    /**
     * @brief Constructs a PlayerInputPacket representing a player's input
     * state.
     *
     * The returned packet has its header type and size initialized and contains
     * the provided input flags and sequence number for ordering.
     *
     * @param input Player input flags (bitmask representing buttons/actions).
     * @param sequence_number Monotonically increasing sequence number for this
     * input.
     * @return PlayerInputPacket Populated packet ready for transmission.
     */
    static PlayerInputPacket makePlayerInput(uint8_t input,
                                             std::uint32_t sequence_number) {
      PlayerInputPacket packet{};
      packet.header.type = PacketType::PlayerInput;
      packet.header.size = sizeof(packet);
      packet.input = input;
      packet.sequence_number = sequence_number;
      return packet;
    }
};
