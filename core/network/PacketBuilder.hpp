#pragma once

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>
#include "Macro.hpp"
#include "Packet.hpp"

struct PacketBuilder {
    /**
     * @brief Constructs a ChatMessagePacket populated with the given text,
     * player ID, RGBA color, and the current timestamp.
     *
     * The provided message is copied into the packet's message buffer; it will
     * be truncated to fit and always null-terminated.
     *
     * @param msg Chat text to include in the packet.
     * @param player_id ID of the player sending the message.
     * @param r Red color component (0-255) for the message.
     * @param g Green color component (0-255) for the message.
     * @param b Blue color component (0-255) for the message.
     * @param a Alpha (opacity) component (0-255) for the message.
     * @return ChatMessagePacket Packet with header.type set to ChatMessage,
     * header.size set to the packet size, timestamp set to the current time,
     * message copied (truncated if necessary and null-terminated), player_id
     * assigned, and color components assigned from the provided RGBA values.
     */
    static ChatMessagePacket makeChatMessage(const std::string &msg,
                                             std::uint32_t player_id,
                                             std::uint8_t r, std::uint8_t g,
                                             std::uint8_t b, std::uint8_t a,
                                             std::uint32_t sequence_number) {
      ChatMessagePacket packet{};
      packet.header.type = PacketType::ChatMessage;
      packet.header.size = sizeof(packet);
      packet.timestamp = static_cast<std::uint32_t>(time(nullptr));
      strncpy(packet.message, msg.c_str(), sizeof(packet.message) - 1);
      packet.message[sizeof(packet.message) - 1] = '\0';
      packet.player_id = player_id;
      packet.sequence_number = sequence_number;
      packet.r = r;
      packet.g = g;
      packet.b = b;
      packet.a = a;
      return packet;
    }

    /**
     * @brief Constructs a ChatMessagePacket with default white color.
     *
     * This overload of makeChatMessage sets the message color to white
     * (RGBA: 255, 255, 255, 255) by default.
     *
     * @param msg Text to include in the message packet; may be truncated to fit
     * the packet.
     * @param player_id Identifier of the player who sent the message.
     * @return ChatMessagePacket Packet with header.type ==
     * PacketType::ChatMessage, header.size set to sizeof(packet), a
     * null-terminated `message` field, current `timestamp`, and `player_id`
     * set. Message color is white.
     */
    static ChatMessagePacket makeChatMessage(const std::string &msg,
                                             std::uint32_t player_id,
                                             std::uint32_t sequence_number) {
      return makeChatMessage(msg, player_id, 255, 255, 255, 255,
                             sequence_number);
    }

    /**
     * @brief Creates a NewPlayerPacket for a newly joined player.
     *
     * The packet's header.type is set to PacketType::NewPlayer and header.size
     * to sizeof(packet).
     *
     * @param player_id Unique identifier for the player.
     * @param player_name Name of the player.
     * @param x Initial X position of the player.
     * @param y Initial Y position of the player.
     * @param speed Initial movement speed of the player.
     * @param max_health Maximum health for the player (default: 100).
     * @param sequence_number Sequence number assigned to the packet (default:
     * 0).
     * @return NewPlayerPacket Populated packet with the provided fields and an
     * initialized header.
     */
    static NewPlayerPacket makeNewPlayer(std::uint32_t player_id,
                                         const std::string &player_name,
                                         float x, float y, float speed,
                                         std::uint32_t sequence_number,
                                         std::uint32_t max_health = 100) {
      NewPlayerPacket packet{};
      packet.header.type = PacketType::NewPlayer;
      packet.header.size = sizeof(packet);
      packet.player_id = player_id;
      strncpy(packet.player_name, player_name.c_str(),
              sizeof(packet.player_name) - 1);
      packet.player_name[sizeof(packet.player_name) - 1] = '\0';
      packet.x = x;
      packet.y = y;
      packet.speed = speed;
      packet.max_health = max_health;
      packet.sequence_number = sequence_number;
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
      packet.header.size = sizeof(packet);
      packet.player_id = player_id;
      packet.sequence_number = seq;
      packet.x = x;
      packet.y = y;
      return packet;
    }

    /**
     * @brief Build a PlayerInfo packet containing the player's display name.
     *
     * The provided name is copied into the packet's fixed-size name field,
     * truncated if necessary, and always null-terminated. The packet's
     * header.type is set to PacketType::PlayerInfo and header.size is set to
     * the packet's sizeof.
     *
     * @param name Player display name to store in the packet.
     * @param sequence_number Sequence number to assign to the packet.
     * @return PlayerInfoPacket Packet with header, name, and sequence_number
     * populated.
     */
    static PlayerInfoPacket makePlayerInfo(const std::string &name,
                                           std::uint32_t sequence_number) {
      PlayerInfoPacket packet{};
      packet.header.type = PacketType::PlayerInfo;
      packet.header.size = sizeof(packet);
      strncpy(packet.name, name.c_str(), sizeof(packet.name) - 1);
      packet.name[sizeof(packet.name) - 1] = '\0';
      packet.sequence_number = sequence_number;
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
     * @brief Create an EnemySpawnPacket containing the enemy's identity, type,
     * position, velocity, health, and sequence metadata.
     *
     * @param sequence_number Sequence number associated with this packet; 0 if
     * not used.
     * @return EnemySpawnPacket Packet with header.type set to EnemySpawn and
     * relevant fields populated.
     */
    static EnemySpawnPacket makeEnemySpawn(uint32_t enemy_id, EnemyType type,
                                           float x, float y, float vx, float vy,
                                           std::uint32_t health,
                                           std::uint32_t max_health,
                                           std::uint32_t sequence_number) {
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
      packet.sequence_number = sequence_number;
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
     * @brief Create an EnemyDeath packet representing an enemy's death and the
     * score awarded to a player.
     *
     * @param enemy_id Identifier of the enemy that died.
     * @param death_x X coordinate where the enemy died.
     * @param death_y Y coordinate where the enemy died.
     * @param player_id Identifier of the player credited with the kill.
     * @param score Score awarded to the player for the kill.
     * @param sequence_number Sequence number associated with this packet.
     * @return EnemyDeathPacket Packet with its fields set (enemy_id, death_x,
     * death_y, player_id, score, sequence_number) and its header configured for
     * PacketType::EnemyDeath with the packet size.
     */
    static EnemyDeathPacket makeEnemyDeath(std::uint32_t enemy_id,
                                           float death_x, float death_y,
                                           std::uint32_t player_id,
                                           std::uint32_t score,
                                           std::uint32_t sequence_number) {
      EnemyDeathPacket packet{};
      packet.header.type = PacketType::EnemyDeath;
      packet.header.size = sizeof(packet);
      packet.enemy_id = enemy_id;
      packet.death_x = death_x;
      packet.death_y = death_y;
      packet.player_id = player_id;
      packet.score = score;
      packet.sequence_number = sequence_number;
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
    static EnemyHitPacket makeEnemyHit(std::uint32_t enemy_id, float hit_x,
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
      packet.header.size = sizeof(packet);
      packet.x = x;
      packet.y = y;
      packet.projectile_type = projectile_type;
      packet.sequence_number = seq;
      return packet;
    }

    /**
     * @brief Create a ProjectileSpawnPacket populated with the given projectile
     * properties and sequence number.
     *
     * @param projectile_id Unique identifier for the projectile.
     * @param type ProjectileType value indicating the projectile kind.
     * @param x Initial x coordinate.
     * @param y Initial y coordinate.
     * @param vel_x Initial velocity along the x axis.
     * @param vel_y Initial velocity along the y axis.
     * @param is_enemy True if fired by an enemy, false if fired by a player.
     * @param damage Damage carried by the projectile.
     * @param owner_id Identifier of the entity that owns or fired the
     * projectile.
     * @param sequence_number Sequence number associated with the packet
     * (default 0).
     * @return ProjectileSpawnPacket Packet populated with the specified fields.
     */
    static ProjectileSpawnPacket makeProjectileSpawn(
        std::uint32_t projectile_id, ProjectileType type, float x, float y,
        float vel_x, float vel_y, bool is_enemy, std::uint32_t damage,
        std::uint32_t owner_id, std::uint32_t sequence_number) {
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
      packet.sequence_number = sequence_number;
      return packet;
    }

    /**
     * @brief Constructs a packet representing a projectile hitting a target.
     *
     * @param projectile_id Identifier of the projectile that hit the target.
     * @param target_id Identifier of the target that was hit.
     * @param hit_x X coordinate where the hit occurred.
     * @param hit_y Y coordinate where the hit occurred.
     * @param target_is_player `1` if the target is a player, `0` otherwise.
     * @return ProjectileHitPacket Packet with header.type set to
     * `ProjectileHit`, header.size set to the packet size, and all hit fields
     * populated.
     */
    static ProjectileHitPacket makeProjectileHit(
        std::uint32_t projectile_id, std::uint32_t target_id, float hit_x,
        float hit_y, std::uint8_t target_is_player) {
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

    /**
     * @brief Construct a ProjectileDestroy packet populated with identifiers
     * and position.
     *
     * @param projectile_id Identifier of the projectile being destroyed.
     * @param x X coordinate where the projectile was destroyed.
     * @param y Y coordinate where the projectile was destroyed.
     * @param sequence_number Sequence number associated with this packet.
     * @return ProjectileDestroyPacket Packet ready for sending with its
     * header.type set to ProjectileDestroy and header.size set to the packet's
     * sizeof.
     */
    static ProjectileDestroyPacket makeProjectileDestroy(
        std::uint32_t projectile_id, float x, float y,
        std::uint32_t sequence_number) {
      ProjectileDestroyPacket packet{};
      packet.header.type = PacketType::ProjectileDestroy;
      packet.header.size = sizeof(packet);
      packet.projectile_id = projectile_id;
      packet.x = x;
      packet.y = y;
      packet.sequence_number = sequence_number;
      return packet;
    }

    /**
     * @brief Constructs a packet that signals the game's start state.
     *
     * Creates a GameStartPacket with the start flag and sequence number
     * populated.
     *
     * @param started `true` if the game has started, `false` otherwise.
     * @param sequence_number Sequence identifier assigned to the packet.
     * @return GameStartPacket Packet containing the game start flag and
     * sequence number.
     */
    static GameStartPacket makeGameStart(bool started,
                                         std::uint32_t sequence_number) {
      GameStartPacket packet{};
      packet.header.type = PacketType::GameStart;
      packet.header.size = sizeof(packet);
      packet.game_start = started;
      packet.sequence_number = sequence_number;
      return packet;
    }

    /**
     * @brief Create a GameEndPacket indicating whether the game has ended.
     *
     * @param ended Whether the game has ended.
     * @param sequence_number Sequence number to assign to the packet (default
     * 0).
     * @return GameEndPacket Packet with header populated, `game_end` set to
     * `ended`, and `sequence_number` set to `sequence_number`.
     */
    static GameEndPacket makeGameEnd(bool ended,
                                     std::uint32_t sequence_number) {
      GameEndPacket packet{};
      packet.header.type = PacketType::GameEnd;
      packet.header.size = sizeof(packet);
      packet.game_end = ended;
      packet.sequence_number = sequence_number;
      return packet;
    }

    /**
     * @brief Create a PlayerDeathPacket describing a player's death location.
     *
     * Constructs a packet populated with the player identifier, world
     * coordinates of the death, and the optional sequence number.
     *
     * @param player_id Identifier of the player who died.
     * @param x World X coordinate of the death location.
     * @param y World Y coordinate of the death location.
     * @param sequence_number Optional sequence number associated with the
     * packet.
     * @return PlayerDeathPacket Packet populated with player_id, x, y, and
     * sequence_number.
     */
    static PlayerDeathPacket makePlayerDeath(std::uint32_t player_id, float x,
                                             float y,
                                             std::uint32_t sequence_number) {
      PlayerDeathPacket packet{};
      packet.header.type = PacketType::PlayerDeath;
      packet.header.size = sizeof(packet);
      packet.player_id = player_id;
      packet.x = x;
      packet.y = y;
      packet.sequence_number = sequence_number;
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
    static PlayerDisconnectPacket makePlayerDisconnect(
        std::uint32_t player_id, std::uint32_t sequence_number) {
      PlayerDisconnectPacket packet{};
      packet.header.type = PacketType::PlayerDisconnected;
      packet.header.size = sizeof(packet);
      packet.player_id = player_id;
      packet.sequence_number = sequence_number;
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
    static HeartbeatPlayerPacket makeHeartbeatPlayer(std::uint32_t player_id) {
      HeartbeatPlayerPacket packet{};
      packet.header.type = PacketType::Heartbeat;
      packet.header.size = sizeof(packet);
      packet.player_id = player_id;
      return packet;
    }

    /**
     * @brief Constructs a CreateRoomPacket populated with the given room
     * parameters.
     *
     * @param room_name Name of the room; will be copied into the packet and
     * truncated if longer than the packet field.
     * @param max_players Maximum number of players allowed in the room.
     * @param password Optional room password. If non-empty the packet is marked
     * private and the password is stored; if empty the password field is
     * cleared.
     * @return CreateRoomPacket Packet ready for sending to request room
     * creation; its header size is set to sizeof(packet) and its type is set to
     * CreateRoom.
     */
    static CreateRoomPacket makeCreateRoom(const std::string &room_name,
                                           std::uint32_t max_players,
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

    /**
     * Constructs a JoinRoomPacket populated with the specified room ID and
     * password.
     *
     * @param room_id Identifier of the room to join.
     * @param password Password for the room; provide an empty string for no
     * password.
     * @return JoinRoomPacket Packet with header.type set to
     * PacketType::JoinRoom, header.size set to the packet size, and the room_id
     * and password fields populated.
     */
    static JoinRoomPacket makeJoinRoom(std::uint32_t room_id,
                                       const std::string &password) {
      JoinRoomPacket packet{};
      packet.header.type = PacketType::JoinRoom;
      packet.header.size = sizeof(packet);
      packet.room_id = room_id;
      strncpy(packet.password, password.c_str(), sizeof(packet.password) - 1);
      packet.password[sizeof(packet.password) - 1] = '\0';
      return packet;
    }

    /**
     * @brief Constructs a JoinRoomResponsePacket with the specified room error
     * code.
     *
     * @param error_code The RoomError value to set in the packet's error_code
     * field.
     * @return JoinRoomResponsePacket Packet with header.type set to
     * JoinRoomResponse, header.size set to the packet size, and error_code set
     * to the provided value.
     */
    static JoinRoomResponsePacket makeJoinRoomResponse(
        const RoomError &error_code) {
      JoinRoomResponsePacket packet{};
      packet.header.type = PacketType::JoinRoomResponse;
      packet.header.size = sizeof(packet);
      packet.error_code = error_code;
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
      packet.header.size = sizeof(packet);
      packet.room_id = room_id;
      return packet;
    }

    /**
     * @brief Creates a ListRoomPacket with its header initialized.
     *
     * @return ListRoomPacket whose header.type is set to PacketType::ListRoom
     * and header.size is set to the packet's byte size.
     */
    static ListRoomPacket makeListRoom() {
      ListRoomPacket packet{};
      packet.header.type = PacketType::ListRoom;
      packet.header.size = sizeof(packet);
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
      packet.header.size = sizeof(packet);
      packet.room_count = static_cast<std::uint32_t>(
          std::min<std::size_t>(rooms.size(), MAX_ROOMS));

      for (std::size_t i = 0; i < packet.room_count; ++i) {
        packet.rooms[i] = rooms[i];
      }
      return packet;
    }

    /**
     * @brief Constructs a MatchmakingRequestPacket initialized with its header.
     *
     * The packet's header type is set to PacketType::MatchmakingRequest and its
     * size is set to the packet's sizeof value.
     *
     * @return MatchmakingRequestPacket packet whose `header.type` is
     * `PacketType::MatchmakingRequest` and `header.size` equals
     * `sizeof(MatchmakingRequestPacket)`.
     */
    static MatchmakingRequestPacket makeMatchmakingRequest() {
      MatchmakingRequestPacket packet{};
      packet.header.type = PacketType::MatchmakingRequest;
      packet.header.size = sizeof(packet);
      return packet;
    }

    /**
     * @brief Create a packet indicating the result of a matchmaking request.
     *
     * @param error_code The RoomError value representing the outcome of the
     * matchmaking request.
     * @return MatchmakingResponsePacket The constructed packet whose header
     * identifies it as a MatchmakingResponse and whose `error_code` field
     * contains the provided `error_code`.
     */
    static MatchmakingResponsePacket makeMatchmakingResponse(
        const RoomError &error_code) {
      MatchmakingResponsePacket packet{};
      packet.header.type = PacketType::MatchmakingResponse;
      packet.header.size = sizeof(packet);
      packet.error_code = error_code;
      return packet;
    };

    /**
     * @brief Construct a PlayerInputPacket containing a player's current input
     * state.
     *
     * The packet's header.type and header.size are initialized; the packet
     * carries the provided input flags and sequence number for ordering.
     *
     * @param input Bitmask of player input flags (buttons/actions).
     * @param sequence_number Monotonically increasing sequence number for this
     * input.
     * @return PlayerInputPacket The populated packet with input flags and
     * sequence number.
     */
    static PlayerInputPacket makePlayerInput(std::uint8_t input,
                                             std::uint32_t sequence_number) {
      PlayerInputPacket packet{};
      packet.header.type = PacketType::PlayerInput;
      packet.header.size = sizeof(packet);
      packet.input = input;
      packet.sequence_number = sequence_number;
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
      packet.header.size = sizeof(packet);
      packet.sequence_number = sequence_number;
      packet.player_id = player_id;
      return packet;
    }
};
