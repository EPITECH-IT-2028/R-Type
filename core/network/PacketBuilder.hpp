#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include "Macro.hpp"
#include "Packet.hpp"
#include "PacketSerialize.hpp"
#include "PacketUtils.hpp"
#include "Serializer.hpp"

struct PacketBuilder {
  private:
    template <typename P>
    /**
     * @brief Compute and assign the packet's header.size from its serialized
     * representation.
     *
     * Serializes a temporary copy of the packet (with header.size cleared) to
     * determine the full serialized size and, on success, sets
     * packet.header.size to that value.
     *
     * @tparam P Packet type containing a modifiable `header.size` and
     * `header.type`.
     * @param packet Packet instance whose `header.size` will be updated on
     * success. On failure the packet is left unchanged.
     * @param ctx Short context string included in error messages when sizing
     * fails.
     * @return true if `packet.header.size` was set to the computed serialized
     * size, `false` if serialization failed or the computed size was outside
     * valid bounds.
     */
    static bool setPayloadSizeFromSerialization(P &packet, const char *ctx) {
      P temp_packet = packet;
      temp_packet.header.size = 0;

      serialization::Buffer serializedBuffer =
          serialization::BitserySerializer::serialize(temp_packet);
      if (serializedBuffer.empty()) {
        std::cerr << "[ERROR] Failed to serialize packet (type: "
                  << packetTypeToString(packet.header.type)
                  << ") for sizing. Context: " << ctx
                  << ". Returning empty packet.\n";
        return false;
      }

      size_t fullSerializedSize = serializedBuffer.size();
      if (fullSerializedSize < HEADER_SIZE) {
        std::cerr << "[ERROR] Serialized size (" << fullSerializedSize
                  << ") is less than minimum valid packet size (" << HEADER_SIZE
                  << " bytes). Context: " << ctx
                  << ". Returning empty packet.\n";
        return false;
      }

      if (fullSerializedSize > std::numeric_limits<std::uint32_t>::max()) {
        std::cerr << "[ERROR] Calculated payload size (" << fullSerializedSize
                  << ") exceeds maximum allowed ("
                  << std::numeric_limits<std::uint32_t>::max()
                  << "). Context: " << ctx << ". Returning empty packet.\n";
        return false;
      }

      packet.header.size = static_cast<std::uint32_t>(fullSerializedSize);
      return true;
    }

    /**
     * @brief Truncates the input string to at most max_bytes bytes.
     *
     * The function returns a new string containing the first up to max_bytes
     * bytes of the provided input. If the input length is less than or equal to
     * max_bytes, the original string is returned unchanged.
     *
     * @param str Input string to truncate.
     * @param max_bytes Maximum number of bytes to keep from the start of `str`.
     * @return std::string The truncated string containing at most `max_bytes`
     * bytes.
     */
    static std::string truncateToBytes(const std::string &str,
                                       size_t max_bytes) {
      return str.length() > max_bytes ? str.substr(0, max_bytes) : str;
    }

  public:
    /**
     * @brief Build a ChatMessagePacket containing a chat message, sender,
     * color, timestamp, and sequence number.
     *
     * Constructs a ChatMessagePacket with header.type set to ChatMessage,
     * timestamp set to the current time, message truncated to at most
     * SERIALIZE_512_BYTES bytes (stored null-terminated), the provided player
     * ID, RGBA color components, and sequence_number. The packet's header.size
     * is computed from the packet's serialization; on sizing failure an
     * empty/default packet is returned by the caller.
     *
     * @param msg Chat text to include in the packet; will be truncated to
     * SERIALIZE_512_BYTES bytes if longer.
     * @param player_id ID of the player sending the message.
     * @param r Red color component (0–255).
     * @param g Green color component (0–255).
     * @param b Blue color component (0–255).
     * @param a Alpha (opacity) component (0–255).
     * @param sequence_number Sequence number assigned to the packet for
     * ordering.
     * @return ChatMessagePacket Packet with populated fields and header.size
     * set to the serialized packet size.
     */
    static ChatMessagePacket makeChatMessage(const std::string &msg,
                                             std::uint32_t player_id,
                                             std::uint8_t r, std::uint8_t g,
                                             std::uint8_t b, std::uint8_t a,
                                             std::uint32_t sequence_number) {
      ChatMessagePacket packet{};
      packet.header.type = PacketType::ChatMessage;
      packet.timestamp = static_cast<std::uint32_t>(time(nullptr));
      packet.message = truncateToBytes(msg, SERIALIZE_512_BYTES);
      packet.player_id = player_id;
      packet.sequence_number = sequence_number;
      packet.r = r;
      packet.g = g;
      packet.b = b;
      packet.a = a;

      if (!setPayloadSizeFromSerialization(packet, "makeChatMessage"))
        return {};
      return packet;
    }

    /**
     * @brief Builds a ChatMessagePacket with white color (RGBA
     * 255,255,255,255).
     *
     * The packet's message is truncated to SERIALIZE_512_BYTES if necessary and
     * timestamped with the current time.
     *
     * @param msg Text to include in the message packet; may be truncated to
     * SERIALIZE_512_BYTES.
     * @param player_id Identifier of the player who sent the message.
     * @param sequence_number Sequence number to include in the packet for
     * ordering.
     * @return ChatMessagePacket Packet with header.type ==
     * PacketType::ChatMessage, message set (null-terminated) and truncated as
     * needed, color set to white, player_id and sequence_number populated, and
     * header.size computed from serialization; returns a default-constructed
     * packet on failure.
     */
    static ChatMessagePacket makeChatMessage(const std::string &msg,
                                             std::uint32_t player_id,
                                             std::uint32_t sequence_number) {
      return makeChatMessage(msg, player_id, 255, 255, 255, 255,
                             sequence_number);
    }

    /**
     * @brief Construct a NewPlayerPacket populated with identity, position,
     * motion, health, and sequence metadata.
     *
     * The provided player_name is truncated to 32 bytes before assignment. The
     * packet's header.size is computed from the serialized payload; if sizing
     * fails an empty/default packet is returned.
     *
     * @param player_id Unique identifier for the player.
     * @param player_name Player display name (will be truncated to 32 bytes).
     * @param x Initial X position.
     * @param y Initial Y position.
     * @param speed Initial movement speed.
     * @param sequence_number Sequence number assigned to this packet for
     * ordering.
     * @param max_health Maximum health for the player (default: 100).
     * @return NewPlayerPacket A packet with the fields set and header
     * populated, or an empty/default packet if payload sizing failed.
     */
    static NewPlayerPacket makeNewPlayer(std::uint32_t player_id,
                                         const std::string &player_name,
                                         float x, float y, float speed,
                                         std::uint32_t sequence_number,
                                         std::uint32_t max_health = 100) {
      NewPlayerPacket packet{};
      packet.header.type = PacketType::NewPlayer;
      packet.player_id = player_id;
      packet.player_name = truncateToBytes(player_name, SERIALIZE_32_BYTES);
      packet.x = x;
      packet.y = y;
      packet.speed = speed;
      packet.sequence_number = sequence_number;
      packet.max_health = max_health;

      if (!setPayloadSizeFromSerialization(packet, "makeNewPlayer"))
        return {};
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
    static PlayerMovePacket makePlayerMove(std::uint32_t player_id,
                                           std::uint32_t seq, float x,
                                           float y) {
      PlayerMovePacket packet{};
      packet.header.type = PacketType::PlayerMove;
      packet.player_id = player_id;
      packet.sequence_number = seq;
      packet.x = x;
      packet.y = y;

      if (!setPayloadSizeFromSerialization(packet, "makePlayerMove"))
        return {};
      return packet;
    }

    /**
     * @brief Create a PlayerInfo packet containing the player's name and
     * sequence number.
     *
     * The provided name is truncated to 32 bytes when stored in the packet.
     *
     * @param name Player name to store in the packet; truncated to 32 bytes if
     * necessary.
     * @param sequence_number Sequence number assigned to the packet.
     * @return PlayerInfoPacket Packet with header.type set to
     * PacketType::PlayerInfo and header.size set to the serialized packet size.
     */
    static PlayerInfoPacket makePlayerInfo(const std::string &name,
                                           std::uint32_t sequence_number) {
      PlayerInfoPacket packet{};
      packet.header.type = PacketType::PlayerInfo;
      packet.name = truncateToBytes(name, 32);
      packet.sequence_number = sequence_number;

      if (!setPayloadSizeFromSerialization(packet, "makePlayerInfo"))
        return {};
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
      packet.player_id = player_id;
      packet.damage = damage;
      packet.x = x;
      packet.y = y;
      packet.sequence_number = sequence_number;

      if (!setPayloadSizeFromSerialization(packet, "makePlayerHit"))
        return {};
      return packet;
    }

    /**
     * @brief Creates an EnemySpawn packet populated with the enemy's identity,
     * type, position, velocity, and health.
     *
     * @param enemy_id Unique identifier for the enemy.
     * @param type Enemy type.
     * @param x Spawn X coordinate.
     * @param y Spawn Y coordinate.
     * @param vx Initial velocity in the X direction.
     * @param vy Initial velocity in the Y direction.
     * @param health Current health value.
     * @param max_health Maximum health value.
     * @param sequence_number Sequence number associated with this packet; 0 if
     * not used.
     * @return EnemySpawnPacket Packet with Header.type set to EnemySpawn and
     * all fields initialized; returns a default-constructed (empty) packet if
     * payload sizing fails.
     */
    static EnemySpawnPacket makeEnemySpawn(uint32_t enemy_id, EnemyType type,
                                           float x, float y, float vx, float vy,
                                           std::uint32_t health,
                                           std::uint32_t max_health,
                                           std::uint32_t sequence_number) {
      EnemySpawnPacket packet{};
      packet.header.type = PacketType::EnemySpawn;
      packet.enemy_id = enemy_id;
      packet.enemy_type = type;
      packet.x = x;
      packet.y = y;
      packet.velocity_x = vx;
      packet.velocity_y = vy;
      packet.health = health;
      packet.max_health = max_health;
      packet.sequence_number = sequence_number;

      if (!setPayloadSizeFromSerialization(packet, "makeEnemySpawn"))
        return {};
      return packet;
    }

    /**
     * @brief Constructs an EnemyMovePacket representing an enemy's position and
     * velocity update.
     *
     * @param enemy_id Unique identifier of the enemy.
     * @param x X coordinate of the enemy's position.
     * @param y Y coordinate of the enemy's position.
     * @param velocity_x Velocity of the enemy along the X axis.
     * @param velocity_y Velocity of the enemy along the Y axis.
     * @param seq Sequence number used to order movement updates.
     * @return EnemyMovePacket Packet with header.type set to
     * `PacketType::EnemyMove`, header.size set to the packet size, and fields
     * populated with the provided values.
     */
    static EnemyMovePacket makeEnemyMove(std::uint32_t enemy_id, float x,
                                         float y, float velocity_x,
                                         float velocity_y, int seq) {
      EnemyMovePacket packet{};
      packet.header.type = PacketType::EnemyMove;
      packet.enemy_id = enemy_id;
      packet.x = x;
      packet.y = y;
      packet.velocity_x = velocity_x;
      packet.velocity_y = velocity_y;
      packet.sequence_number = seq;

      if (!setPayloadSizeFromSerialization(packet, "makeEnemyMove"))
        return {};
      return packet;
    }

    /**
     * @brief Create an EnemyDeath packet recording an enemy's death and awarded
     * score.
     *
     * @param enemy_id ID of the enemy that died.
     * @param death_x X coordinate of the death location.
     * @param death_y Y coordinate of the death location.
     * @param player_id ID of the player credited with the kill.
     * @param score Score awarded to the player.
     * @param sequence_number Sequence number for ordering this packet.
     * @return EnemyDeathPacket Packet with `header.type` set to
     * `PacketType::EnemyDeath`, fields populated, and `header.size` set to the
     * computed serialized size on success; returns a default-constructed packet
     * if size computation fails.
     */
    static EnemyDeathPacket makeEnemyDeath(std::uint32_t enemy_id,
                                           float death_x, float death_y,
                                           std::uint32_t player_id,
                                           std::uint32_t score,
                                           std::uint32_t sequence_number) {
      EnemyDeathPacket packet{};
      packet.header.type = PacketType::EnemyDeath;
      packet.enemy_id = enemy_id;
      packet.death_x = death_x;
      packet.death_y = death_y;
      packet.player_id = player_id;
      packet.score = score;
      packet.sequence_number = sequence_number;

      if (!setPayloadSizeFromSerialization(packet, "makeEnemyDeath"))
        return {};
      return packet;
    }

    /**
     * @brief Create an EnemyHit packet describing damage inflicted on an enemy.
     *
     * @param enemy_id Identifier of the enemy that was hit.
     * @param hit_x X coordinate of the hit location in world space.
     * @param hit_y Y coordinate of the hit location in world space.
     * @param damage Damage amount applied to the enemy.
     * @param sequence_number Sequence number used to order this event.
     * @return EnemyHitPacket Packet with header.type set to EnemyHit and fields
     * populated: enemy_id, hit_x, hit_y, damage, and sequence_number.
     */
    static EnemyHitPacket makeEnemyHit(std::uint32_t enemy_id, float hit_x,
                                       float hit_y, float damage,
                                       int sequence_number) {
      EnemyHitPacket packet{};
      packet.header.type = PacketType::EnemyHit;
      packet.enemy_id = enemy_id;
      packet.hit_x = hit_x;
      packet.hit_y = hit_y;
      packet.damage = damage;
      packet.sequence_number = sequence_number;

      if (!setPayloadSizeFromSerialization(packet, "makeEnemyHit"))
        return {};
      return packet;
    }

    /**
     * @brief Constructs a PlayerShootPacket with shot position, projectile
     * type, and sequence number.
     *
     * @param x X coordinate of the shot.
     * @param y Y coordinate of the shot.
     * @param projectile_type Projectile type fired.
     * @param seq Sequence number used to order the shot.
     * @return PlayerShootPacket Packet populated with position, projectile
     * type, and sequence number.
     */
    static PlayerShootPacket makePlayerShoot(float x, float y,
                                             ProjectileType projectile_type,
                                             std::uint32_t seq) {
      PlayerShootPacket packet{};
      packet.header.type = PacketType::PlayerShoot;
      packet.x = x;
      packet.y = y;
      packet.projectile_type = projectile_type;
      packet.sequence_number = seq;

      if (!setPayloadSizeFromSerialization(packet, "makePlayerShoot"))
        return {};
      return packet;
    }

    /**
     * @brief Builds a ProjectileSpawnPacket with the supplied identity, type,
     * kinematic and ownership properties and computes its serialized payload
     * size.
     *
     * @param projectile_id Unique identifier for the projectile.
     * @param type ProjectileType value specifying the projectile kind.
     * @param x Initial x coordinate.
     * @param y Initial y coordinate.
     * @param vel_x Initial velocity along the x axis.
     * @param vel_y Initial velocity along the y axis.
     * @param is_enemy True if fired by an enemy, false if fired by a player.
     * @param damage Damage carried by the projectile.
     * @param owner_id Identifier of the entity that owns or fired the
     * projectile.
     * @param sequence_number Sequence number associated with the packet
     * @return ProjectileSpawnPacket Packet populated with header.type, fields
     * above, and header.size set based on serialization; returns a
     * default-constructed (empty) packet if payload sizing fails.
     */
    static ProjectileSpawnPacket makeProjectileSpawn(
        std::uint32_t projectile_id, ProjectileType type, float x, float y,
        float vel_x, float vel_y, bool is_enemy, std::uint32_t damage,
        std::uint32_t owner_id, std::uint32_t sequence_number) {
      ProjectileSpawnPacket packet{};
      packet.header.type = PacketType::ProjectileSpawn;
      packet.projectile_id = projectile_id;
      packet.projectile_type = type;
      packet.x = x;
      packet.y = y;
      packet.velocity_x = vel_x;
      packet.velocity_y = vel_y;
      packet.is_enemy_projectile = is_enemy;
      packet.damage = damage;
      packet.owner_id = owner_id;
      packet.sequence_number = sequence_number;

      if (!setPayloadSizeFromSerialization(packet, "makeProjectileSpawn"))
        return {};
      return packet;
    }

    /**
     * @brief Create a packet representing a projectile striking a target.
     *
     * @param projectile_id ID of the projectile that caused the hit.
     * @param target_id ID of the entity that was hit.
     * @param hit_x X coordinate of the hit location.
     * @param hit_y Y coordinate of the hit location.
     * @param target_is_player `1` if the target is a player, `0` otherwise.
     * @return ProjectileHitPacket Packet with `header.type` set to
     * `ProjectileHit`, hit fields populated, and `header.size` computed from
     * serialization; returns an empty/default packet if sizing fails.
     */
    static ProjectileHitPacket makeProjectileHit(
        std::uint32_t projectile_id, std::uint32_t target_id, float hit_x,
        float hit_y, std::uint8_t target_is_player) {
      ProjectileHitPacket packet{};
      packet.header.type = PacketType::ProjectileHit;
      packet.projectile_id = projectile_id;
      packet.target_id = target_id;
      packet.hit_x = hit_x;
      packet.hit_y = hit_y;
      packet.target_is_player = target_is_player;

      if (!setPayloadSizeFromSerialization(packet, "makeProjectileHit"))
        return {};
      return packet;
    }

    /**
     * @brief Create a ProjectileDestroyPacket populated with projectile id,
     * position, and sequence number.
     *
     * @param projectile_id Identifier of the projectile.
     * @param x X coordinate where the projectile was destroyed.
     * @param y Y coordinate where the projectile was destroyed.
     * @param sequence_number Sequence number associated with this packet.
     * @return ProjectileDestroyPacket The packet with its fields set and
     * `header.type` set to `PacketType::ProjectileDestroy`. The function does
     * not modify `header.size`.
     */
    static ProjectileDestroyPacket makeProjectileDestroy(
        std::uint32_t projectile_id, float x, float y,
        std::uint32_t sequence_number) {
      ProjectileDestroyPacket packet{};
      packet.header.type = PacketType::ProjectileDestroy;
      packet.projectile_id = projectile_id;
      packet.x = x;
      packet.y = y;
      packet.sequence_number = sequence_number;
      if (!setPayloadSizeFromSerialization(packet, "makeProjectileDestroy"))
        return {};
      return packet;
    }

    /**
     * @brief Create a GameStartPacket that records whether the game has
     * started.
     *
     * @param started Whether the game has started.
     * @param sequence_number Sequence identifier assigned to the packet.
     * @return GameStartPacket Packet with header.type set to GameStart,
     * header.size computed, and the `game_start` and `sequence_number` fields
     * populated.
     */
    static GameStartPacket makeGameStart(bool started,
                                         std::uint32_t sequence_number) {
      GameStartPacket packet{};
      packet.header.type = PacketType::GameStart;
      packet.header.size = sizeof(packet);
      packet.game_start = started;
      packet.sequence_number = sequence_number;

      if (!setPayloadSizeFromSerialization(packet, "makeGameStart"))
        return {};
      return packet;
    }

    /**
     * @brief Construct a GameEnd packet indicating the game's end state.
     *
     * @param ended Whether the game has ended.
     * @param sequence_number Packet sequence number for ordering.
     * @return GameEndPacket Packet with `header.type` set to
     * `PacketType::GameEnd`, `game_end` set to `ended`, and `sequence_number`
     * populated.
     */
    static GameEndPacket makeGameEnd(bool ended,
                                     std::uint32_t sequence_number) {
      GameEndPacket packet{};
      packet.header.type = PacketType::GameEnd;
      packet.game_end = ended;
      packet.sequence_number = sequence_number;

      if (!setPayloadSizeFromSerialization(packet, "makeGameEnd"))
        return {};
      return packet;
    }

    /**
     * @brief Build a PlayerDeathPacket describing a player's death position and
     * sequence.
     *
     * Constructs a packet with PacketType::PlayerDeath, sets player identifier,
     * world coordinates, and sequence number, then computes and assigns the
     * serialized payload size; returns a default-constructed packet if payload
     * sizing fails.
     *
     * @param player_id Identifier of the player who died.
     * @param x World X coordinate of the death location.
     * @param y World Y coordinate of the death location.
     * @param sequence_number Sequence number associated with this packet.
     * @return PlayerDeathPacket Populated packet with header.size set on
     * success, a default-constructed (empty) packet on failure.
     */
    static PlayerDeathPacket makePlayerDeath(std::uint32_t player_id, float x,
                                             float y,
                                             std::uint32_t sequence_number) {
      PlayerDeathPacket packet{};
      packet.header.type = PacketType::PlayerDeath;
      packet.player_id = player_id;
      packet.x = x;
      packet.y = y;
      packet.sequence_number = sequence_number;

      if (!setPayloadSizeFromSerialization(packet, "makePlayerDeath"))
        return {};
      return packet;
    }

    /**
     * @brief Builds a PlayerDisconnect packet for a disconnected player.
     *
     * @param player_id ID of the disconnected player.
     * @param sequence_number Packet sequence number for ordering.
     * @return PlayerDisconnectPacket Packet with header.type set to
     * PacketType::PlayerDisconnected, player_id and sequence_number populated,
     * and header.size set to the computed serialized size. Returns a
     * default-constructed packet if payload sizing fails.
     */
    static PlayerDisconnectPacket makePlayerDisconnect(
        std::uint32_t player_id, std::uint32_t sequence_number) {
      PlayerDisconnectPacket packet{};
      packet.header.type = PacketType::PlayerDisconnected;
      packet.player_id = player_id;
      packet.sequence_number = sequence_number;

      if (!setPayloadSizeFromSerialization(packet, "makePlayerDisconnect"))
        return {};
      return packet;
    }

    /**
     * @brief Build a heartbeat packet for the given player.
     *
     * @param player_id Player identifier to include in the packet.
     * @return HeartbeatPlayerPacket with header.type set to
     * PacketType::Heartbeat, header.size set based on serialization, and
     * player_id set to the provided value.
     */
    static HeartbeatPlayerPacket makeHeartbeatPlayer(std::uint32_t player_id) {
      HeartbeatPlayerPacket packet{};
      packet.header.type = PacketType::Heartbeat;
      packet.player_id = player_id;

      if (!setPayloadSizeFromSerialization(packet, "makeHeartbeatPlayer"))
        return {};
      return packet;
    }

    /**
     * @brief Create a CreateRoomPacket initialized with the provided room
     * parameters.
     *
     * The packet's header.type is set to CreateRoom, string fields are
     * truncated to fit payload limits (room_name truncated to 32 bytes;
     * password truncated to 32 bytes when set), and header.size is computed
     * from the packet's serialized size.
     *
     * @param room_name Name of the room; will be copied and truncated to 32
     * bytes if longer.
     * @param max_players Maximum number of players allowed in the room.
     * @param password Optional room password; if non-empty the packet is marked
     * private and the truncated password is stored, otherwise the password
     * field is cleared.
     * @return CreateRoomPacket Packet populated for a create-room request with
     * header.size set to the computed serialized size.
     */
    static CreateRoomPacket makeCreateRoom(const std::string &room_name,
                                           std::uint32_t max_players,
                                           std::uint32_t sequence_number,
                                           const std::string &password = "") {
      CreateRoomPacket packet{};
      packet.header.type = PacketType::CreateRoom;
      packet.room_name = truncateToBytes(room_name, 32);
      packet.max_players = max_players;
      packet.is_private = !password.empty();
      packet.sequence_number = sequence_number;
      if (packet.is_private)
        packet.password = password;
      else
        packet.password.clear();

      if (!setPayloadSizeFromSerialization(packet, "makeCreateRoom"))
        return {};
      return packet;
    }

    /**
     * @brief Builds a CreateRoomResponsePacket indicating the result of a
     * create-room request.
     *
     * @param error_code Result code to set in the packet.
     * @param room_id Identifier of the created room (or 0 when not applicable).
     * @param sequence_number Sequence number for ordering the packet.
     * @return CreateRoomResponsePacket Packet with header.type set to
     * CreateRoomResponse, error_code, room_id and sequence_number populated,
     * and header.size computed from serialization.
     */
    static CreateRoomResponsePacket makeCreateRoomResponse(
        const RoomError &error_code, std::uint32_t room_id,
        std::uint32_t sequence_number) {
      CreateRoomResponsePacket packet{};
      packet.header.type = PacketType::CreateRoomResponse;
      packet.error_code = error_code;
      packet.room_id = room_id;
      packet.sequence_number = sequence_number;

      if (!setPayloadSizeFromSerialization(packet, "makeCreateRoomResponse"))
        return {};
      return packet;
    }

    /**
     * @brief Creates a JoinRoomPacket for joining a specific room.
     *
     * Builds a packet with the specified room ID, password, and sequence
     * number, sets the packet type to PacketType::JoinRoom, and
     * computes/assigns the serialized payload size to header.size.
     *
     * @param room_id ID of the room to join.
     * @param password Password for the room; empty string means no password. If
     * the password exceeds the allowed size it is truncated to 32 bytes.
     * @param sequence_number Sequence number to include in the packet.
     * @return JoinRoomPacket Packet with header.type set to JoinRoom,
     * header.size set to the computed serialized size, and room_id, password,
     * and sequence_number populated.
     */
    static JoinRoomPacket makeJoinRoom(std::uint32_t room_id,
                                       const std::string &password,
                                       std::uint32_t sequence_number) {
      JoinRoomPacket packet{};
      packet.header.type = PacketType::JoinRoom;
      packet.room_id = room_id;
      packet.password = password;
      packet.sequence_number = sequence_number;

      if (!setPayloadSizeFromSerialization(packet, "makeJoinRoom"))
        return {};
      return packet;
    }

    /**
     * @brief Builds a JoinRoomResponsePacket populated with the provided error
     * code and sequence number.
     *
     * @param error_code RoomError to store in the packet's error_code field.
     * @param sequence_number Sequence number to store in the packet's
     * sequence_number field.
     * @return JoinRoomResponsePacket Packet with header.type set to
     * JoinRoomResponse and header.size set to the computed serialized size.
     */
    static JoinRoomResponsePacket makeJoinRoomResponse(
        const RoomError &error_code, std::uint32_t sequence_number) {
      JoinRoomResponsePacket packet{};
      packet.header.type = PacketType::JoinRoomResponse;
      packet.error_code = error_code;
      packet.sequence_number = sequence_number;

      if (!setPayloadSizeFromSerialization(packet, "makeJoinRoomResponse"))
        return {};
      return packet;
    }

    /**
     * @brief Create a LeaveRoomPacket for the given room identifier.
     *
     * @param room_id Identifier of the room to leave.
     * @return LeaveRoomPacket with its header type set to `LeaveRoom`, header
     * size set to the packet size, and `room_id` set to the provided value.
     */
    static LeaveRoomPacket makeLeaveRoom(std::uint32_t room_id) {
      LeaveRoomPacket packet{};
      packet.header.type = PacketType::LeaveRoom;
      packet.room_id = room_id;

      if (!setPayloadSizeFromSerialization(packet, "makeLeaveRoom"))
        return {};
      return packet;
    }

    /**
     * @brief Builds a packet requesting the server's room list.
     *
     * @return ListRoomPacket whose header.type is PacketType::ListRoom and
     * whose header.size equals the packet's serialized byte size; returns a
     * default-constructed (empty) packet if sizing fails.
     */
    static ListRoomPacket makeListRoom() {
      ListRoomPacket packet{};
      packet.header.type = PacketType::ListRoom;

      if (!setPayloadSizeFromSerialization(packet, "makeListRoom"))
        return {};
      return packet;
    }

    /**
     * @brief Construct a ListRoomResponsePacket populated from a vector of
     * rooms.
     *
     * @param rooms Vector of RoomInfo entries to include in the response; up to
     * MAX_ROOMS entries are copied.
     * @return ListRoomResponsePacket Packet whose header type is set to
     * ListRoomResponse, header size is set, and whose room_count is the minimum
     * of rooms.size() and MAX_ROOMS with the first room_count entries copied
     * into packet.rooms.
     */
    static ListRoomResponsePacket makeListRoomResponse(
        const std::vector<RoomInfo> &rooms) {
      ListRoomResponsePacket packet{};
      packet.header.type = PacketType::ListRoomResponse;
      packet.room_count = static_cast<std::uint32_t>(
          std::min<std::size_t>(rooms.size(), MAX_ROOMS));

      for (std::size_t i = 0; i < packet.room_count; ++i) {
        packet.rooms[i] = rooms[i];
      }

      if (!setPayloadSizeFromSerialization(packet, "makeListRoomResponse"))
        return {};
      return packet;
    }

    /**
     * @brief Construct a MatchmakingRequestPacket with header.type set and
     * sequence number assigned.
     *
     * @param sequence_number Sequence number for packet ordering.
     * @return MatchmakingRequestPacket whose header.type is
     * PacketType::MatchmakingRequest and whose header.size is set to the
     * packet's serialized size; an empty default-constructed packet is returned
     * if payload sizing fails.
     */
    static MatchmakingRequestPacket makeMatchmakingRequest(
        std::uint32_t sequence_number) {
      MatchmakingRequestPacket packet{};
      packet.header.type = PacketType::MatchmakingRequest;
      packet.sequence_number = sequence_number;

      if (!setPayloadSizeFromSerialization(packet, "makeMatchmakingRequest"))
        return {};
      return packet;
    }

    /**
     * @brief Construct a MatchmakingResponsePacket representing the result of a
     * matchmaking request.
     *
     * @param error_code Matchmaking result code.
     * @param sequence_number Sequence number for the packet (used for
     * ordering).
     * @return MatchmakingResponsePacket Packet with header.type set to
     * MatchmakingResponse, `error_code` and `sequence_number` assigned, and
     * header.size set to the computed serialized size; returns an empty/default
     * packet if payload sizing fails.
     */
    static MatchmakingResponsePacket makeMatchmakingResponse(
        const RoomError &error_code, std::uint32_t sequence_number) {
      MatchmakingResponsePacket packet{};
      packet.header.type = PacketType::MatchmakingResponse;
      packet.error_code = error_code;
      packet.sequence_number = sequence_number;

      if (!setPayloadSizeFromSerialization(packet, "makeMatchmakingResponse"))
        return {};
      return packet;
    };

    /**
     * @brief Create a PlayerInputPacket representing a player's input state.
     *
     * The packet's header.type is set to PlayerInput and header.size is
     * computed from the serialized packet; if size computation fails an empty
     * packet is returned.
     *
     * @param input Bitmask of player input flags (buttons/actions).
     * @param sequence_number Monotonically increasing sequence number for
     * ordering inputs.
     * @return PlayerInputPacket The populated packet with input flags and
     * sequence number, or an empty packet on sizing failure.
     */
    static PlayerInputPacket makePlayerInput(std::uint8_t input,
                                             std::uint32_t sequence_number) {
      PlayerInputPacket packet{};
      packet.header.type = PacketType::PlayerInput;
      packet.input = input;
      packet.sequence_number = sequence_number;

      if (!setPayloadSizeFromSerialization(packet, "makePlayerInput"))
        return {};
      return packet;
    }

    /**
     * @brief Constructs an acknowledgement packet for a specific sequence and
     * player.
     *
     * @param sequence_number Sequence number being acknowledged.
     * @param player_id Identifier of the player associated with this
     * acknowledgement.
     * @return AckPacket Packet with its header set to `PacketType::Ack`,
     * `header.size` set, and `sequence_number` and `player_id` populated.
     */
    static AckPacket makeAckPacket(uint32_t sequence_number,
                                   std::uint32_t player_id) {
      AckPacket packet{};
      packet.header.type = PacketType::Ack;
      packet.sequence_number = sequence_number;
      packet.player_id = player_id;

      if (!setPayloadSizeFromSerialization(packet, "makeAckPacket"))
        return {};
      return packet;
    }

    /**
     * @brief Constructs a RequestChallengePacket for a specific room.
     *
     * @param room_id Identifier of the room to request a challenge for.
     * @param sequence_number Sequence number associated with the packet (used
     * for ordering).
     * @return RequestChallengePacket Packet with `header.type` set to
     * `PacketType::RequestChallenge`, `room_id` and `sequence_number`
     * populated, and `header.size` computed from serialization. Returns an
     * empty/default packet if payload sizing via serialization fails.
     */
    static RequestChallengePacket makeRequestChallenge(
        std::uint32_t room_id, std::uint32_t sequence_number) {
      RequestChallengePacket packet{};
      packet.header.type = PacketType::RequestChallenge;
      packet.room_id = room_id;
      packet.sequence_number = sequence_number;

      if (!setPayloadSizeFromSerialization(packet, "makeRequestChallenge"))
        return {};
      return packet;
    }

    /**
     * @brief Create a ChallengeResponsePacket populated with the provided
     * challenge, timestamp, and sequence number.
     *
     * @param challenge Hex-encoded challenge string to store in the packet;
     * expected length is CHALLENGE_HEX_LEN characters.
     * @param timestamp Timestamp associated with the challenge response.
     * @param sequence_number Sequence number assigned to the packet.
     * @return ChallengeResponsePacket Packet whose header.type is
     * PacketType::ChallengeResponse, whose payload fields (challenge,
     * timestamp, sequence_number) are set, and whose header.size is computed
     * from the serialized packet.
     */
    static ChallengeResponsePacket makeChallengeResponse(
        const std::string challenge, std::uint32_t timestamp,
        std::uint32_t sequence_number) {
      ChallengeResponsePacket packet{};
      packet.header.type = PacketType::ChallengeResponse;
      packet.challenge = challenge;
      packet.timestamp = timestamp;
      packet.sequence_number = sequence_number;

      if (!setPayloadSizeFromSerialization(packet, "makeChallengeResponse"))
        return {};
      return packet;
    }

    /**
     * @brief Constructs a PingPacket with the specified timestamp.
     *
     * @param timestamp Timestamp to include in the ping packet.
     * @return PingPacket Packet with header.type set to Ping, header.size set
     * to the packet size, and timestamp set to the provided value.
     */
    static PingPacket makePing(std::uint32_t timestamp) {
      PingPacket packet{};
      packet.header.type = PacketType::Ping;
      packet.timestamp = timestamp;

      if (!setPayloadSizeFromSerialization(packet, "makePing"))
        return {};;
      return packet;
    }

    /**
     * @brief Creates a ScoreboardRequest packet to fetch top scores.
     *
     * @param limit Maximum number of scores to retrieve (e.g., 10 for top 10).
     * @return ScoreboardRequestPacket with header.type set to
     * ScoreboardRequest.
     */
    static ScoreboardRequestPacket makeScoreboardRequest(
        std::uint32_t limit = 10) {
      ScoreboardRequestPacket packet{};
      packet.header.type = PacketType::ScoreboardRequest;
      packet.limit = limit;

      if (!setPayloadSizeFromSerialization(packet, "makeScoreboardRequest"))
        return {};
      return packet;
    }

    /**
     * @brief Constructs a PongPacket with the specified timestamp.
     *
     * @param timestamp Timestamp to include in the pong packet.
     * @return PongPacket Packet with header.type set to Pong, header.size set
     * to the packet size, and timestamp set to the provided value.
     */
    static PongPacket makePong(std::uint32_t timestamp) {
      PongPacket packet{};
      packet.header.type = PacketType::Pong;
      packet.timestamp = timestamp;
      
      if (!setPayloadSizeFromSerialization(packet, "makePong"))
        return {};
      return packet;
    }

    /**
     * @brief Creates a ScoreboardResponse packet with player scores.
     *
     * @param scores Vector of ScoreEntry containing player names and scores.
     * @return ScoreboardResponsePacket with scores sorted by highest score
     * first.
     */
    static ScoreboardResponsePacket makeScoreboardResponse(
        const std::vector<ScoreEntry> &scores) {
      ScoreboardResponsePacket packet{};
      packet.header.type = PacketType::ScoreboardResponse;

      if (scores.size() > std::numeric_limits<std::uint32_t>::max()) {
        std::cerr << "Error: Too many scores to fit in ScoreboardResponsePacket"
                  << std::endl;
        return {};
      }

      packet.entry_count = static_cast<std::uint32_t>(scores.size());
      packet.scores = scores;

      if (!setPayloadSizeFromSerialization(packet, "makeScoreboardResponse"))
        return {};
      return packet;
    }
};
