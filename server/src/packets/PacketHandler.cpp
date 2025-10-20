#include "PacketHandler.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include "Broadcast.hpp"
#include "GameManager.hpp"
#include "Macro.hpp"
#include "Packet.hpp"
#include "PacketSerialize.hpp"
#include "Server.hpp"

void packet::ResponseHelper::sendJoinRoomResponse(server::Server &server,
                                                  std::uint32_t player_id,
                                                  RoomError error) {
  auto response = PacketBuilder::makeJoinRoomResponse(error);
  auto serializedBuffer = serialization::BitserySerializer::serialize(response);
  server.getNetworkManager().sendToClient(
      player_id, reinterpret_cast<const char *>(serializedBuffer.data()),
      serializedBuffer.size());
}

void packet::ResponseHelper::sendMatchmakingResponse(server::Server &server,
                                                     std::uint32_t player_id,
                                                     RoomError error) {
  auto response = PacketBuilder::makeMatchmakingResponse(error);
  auto serializedBuffer = serialization::BitserySerializer::serialize(response);
  server.getNetworkManager().sendToClient(
      player_id, reinterpret_cast<const char *>(serializedBuffer.data()),
      serializedBuffer.size());
}

int packet::MessageHandler::handlePacket(server::Server &server,
                                         server::Client &client,
                                         const char *data, std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<MessagePacket>(buffer);

  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize MessagePacket from client "
              << client._player_id << std::endl;
    return KO;
  }
  const MessagePacket &packet = deserializedPacket.value();
  std::cout << "[MESSAGE] Player " << client._player_id << ": "
            << packet.message << std::endl;

  MessagePacket validatedPacket = packet;
  validatedPacket.player_id = static_cast<std::uint32_t>(client._player_id);

  auto room = server.getGameManager().getRoom(client._room_id);
  if (!room) {
    std::cerr << "[ERROR] Client " << client._player_id << " is not in any room"
              << std::endl;
    return KO;
  }

  auto roomClients = room->getClients();

  broadcast::Broadcast::broadcastMessageToRoom(server.getNetworkManager(),
                                               roomClients, validatedPacket);
  return OK;
}

/**
 * @brief Handles an incoming PlayerInfoPacket from a client, registers the
 * player in their room, sends the player's info back to the originating client,
 * and broadcasts existing and new-player information to room participants.
 *
 * @param server Server instance managing rooms, game state, and networking.
 * @param client Client that sent the packet; its player and entity IDs may be
 * updated.
 * @param data Pointer to the raw packet bytes containing a PlayerInfoPacket.
 * @param size Size of the data buffer in bytes.
 * @return int `OK` on successful handling (player created/broadcasts sent),
 * `KO` on failure such as deserialization error or missing room.
 */
int packet::PlayerInfoHandler::handlePacket(server::Server &server,
                                            server::Client &client,
                                            const char *data,
                                            std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<PlayerInfoPacket>(buffer);

  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize PlayerInfoPacket from client "
              << client._player_id << std::endl;
    return KO;
  }

  const PlayerInfoPacket &packet = deserializedPacket.value();

  // Ensure null-termination of the name
  char nameBuf[sizeof(packet.name) + 1];
  std::memcpy(nameBuf, packet.name, sizeof(packet.name));
  nameBuf[sizeof(packet.name)] = '\0';
  std::string name(nameBuf);

  client._player_name = name;

  switch (client._state) {
    case server::ClientState::CONNECTED_MENU:
      std::cout << "[PLAYERINFO] Client " << client._player_id << " (" << name
                << ") registered in menu" << std::endl;
      return OK;
    case server::ClientState::IN_ROOM_WAITING:
    case server::ClientState::IN_GAME:
      return server.initializePlayerInRoom(client) ? OK : KO;

    default:
      std::cerr << "[ERROR] Invalid client state for PlayerInfo" << std::endl;
      return KO;
  }
}

/**
 * @brief Validates a HeartbeatPlayerPacket and updates the client's heartbeat
 * timestamp.
 *
 * Validates that the incoming packet deserializes correctly and that its
 * player_id matches the client's player id; on success updates the client's
 * last_heartbeat to the current time.
 *
 * @param client The client whose heartbeat is being validated and updated.
 * @param data Pointer to the received packet data.
 * @param size Size of the received packet data in bytes.
 * @return int `OK` on successful validation and heartbeat update, `KO`
 * otherwise.
 */
int packet::HeartbeatPlayerHandler::handlePacket(
    [[maybe_unused]] server::Server &server, server::Client &client,
    const char *data, std::size_t size) {
  serialization::Buffer buffer(data, data + size);
  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<HeartbeatPlayerPacket>(
          buffer);
  if (!deserializedPacket) {
    std::cerr
        << "[ERROR] Failed to deserialize HeartbeatPlayerPacket from client "
        << client._player_id << std::endl;
    return KO;
  }

  const auto &hb = deserializedPacket.value();
  if (hb.player_id != static_cast<std::uint32_t>(client._player_id)) {
    return KO;
  }
  client._last_heartbeat = std::chrono::steady_clock::now();
  return OK;
}

int packet::PlayerShootHandler::handlePacket(server::Server &server,
                                             server::Client &client,
                                             const char *data,
                                             std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<PlayerShootPacket>(buffer);

  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize PlayerShootPacket from client "
              << client._player_id << std::endl;
    return KO;
  }

  const PlayerShootPacket &packet = deserializedPacket.value();

  auto room = server.getGameManager().getRoom(client._room_id);
  if (!room) {
    return KO;
  }

  auto player = room->getGame().getPlayer(client._player_id);
  if (!player) {
    return KO;
  }

  std::pair<float, float> pos = player->getPosition();
  const float speed = PROJECTILE_SPEED;

  float vx = speed;
  float vy = 0.0f;

  std::uint32_t projectileId = room->getGame().getNextProjectileId();

  auto projectileType = packet.projectile_type;

  if (projectileType != ProjectileType::PLAYER_BASIC) {
    projectileType = ProjectileType::PLAYER_BASIC;
  }

  auto projectile = room->getGame().createProjectile(
      projectileId, client._player_id, projectileType, pos.first, pos.second,
      vx, vy);

  if (!projectile) {
    return KO;
  }

  auto playerShotPacket = PacketBuilder::makePlayerShoot(
      pos.first, pos.second, projectileType, packet.sequence_number);

  auto roomClients = room->getClients();
  broadcast::Broadcast::broadcastPlayerShootToRoom(
      server.getNetworkManager(), roomClients, playerShotPacket);

  return OK;
}

/**
 * @brief Handle an incoming PlayerDisconnectPacket and process the player's
 * disconnection.
 *
 * Processes a serialized PlayerDisconnectPacket from the provided buffer,
 * validates the packet's player id against the client, updates server and
 * client state, removes the player from their room and game (if present),
 * broadcasts the disconnect to remaining room clients, and clears the client's
 * server slot.
 *
 * @param data Pointer to the serialized PlayerDisconnectPacket.
 * @param size Size of the serialized data in bytes.
 * @return int `OK` on successful processing; `KO` if deserialization fails or
 * the packet's player_id does not match the client.
 */
int packet::PlayerDisconnectedHandler::handlePacket(server::Server &server,
                                                    server::Client &client,
                                                    const char *data,
                                                    std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<PlayerDisconnectPacket>(
          buffer);

  if (!deserializedPacket) {
    std::cerr
        << "[ERROR] Failed to deserialize PlayerDisconnectPacket from client "
        << client._player_id << std::endl;
    return KO;
  }

  const auto &disc = deserializedPacket.value();
  if (disc.player_id != static_cast<std::uint32_t>(client._player_id)) {
    return KO;
  }
  std::cout << "[WORLD] Player " << client._player_id << " disconnected."
            << std::endl;

  bool wasConnected = client._connected;
  client._connected = false;

  if (wasConnected) {
    server.setPlayerCount(server.getPlayerCount() - 1);
  }

  if (client._room_id != NO_ROOM) {
    auto room = server.getGameManager().getRoom(client._room_id);
    if (room) {
      auto player = room->getGame().getPlayer(client._player_id);
      if (player) {
        room->getGame().destroyPlayer(client._player_id);
      }

      std::shared_ptr<server::Client> sharedClient;
      for (const auto &entry : server.getClients()) {
        if (entry && entry->_player_id == client._player_id) {
          sharedClient = entry;
          break;
        }
      }
      if (sharedClient) {
        server.getGameManager().leaveRoom(sharedClient);
      }
      auto roomClients = room->getClients();

      auto disconnectPacket =
          PacketBuilder::makePlayerDisconnect(client._player_id);
      broadcast::Broadcast::broadcastPlayerDisconnectToRoom(
          server.getNetworkManager(), roomClients, disconnectPacket);
    }
  }
  server.clearClientSlot(client._player_id);
  return OK;
}

int packet::CreateRoomHandler::handlePacket(server::Server &server,
                                            server::Client &client,
                                            const char *data,
                                            std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<CreateRoomPacket>(buffer);

  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize CreateRoomPacket from client "
              << client._player_id << std::endl;
    return KO;
  }

  const CreateRoomPacket &packet = deserializedPacket.value();

  const size_t roomLen = strnlen(packet.room_name, sizeof(packet.room_name));
  const size_t passLen = strnlen(packet.password, sizeof(packet.password));

  std::string roomName(packet.room_name, roomLen);
  std::string password(packet.password, passLen);
  auto newRoom = server.getGameManager().createRoom(roomName, password);

  if (!newRoom) {
    std::cerr << "[ERROR] Failed to create room for client "
              << client._player_id << std::endl;
    return KO;
  }

  auto sharedClient = server.getClientById(client._player_id);
  if (!sharedClient) {
    std::cerr << "[ERROR] Failed to get shared_ptr for client "
              << client._player_id << std::endl;
    server.getGameManager().destroyRoom(newRoom->getRoomId());
    return KO;
  }

  bool joinSuccess = server.getGameManager().joinRoom(
      newRoom->getRoomId(), server.getClientById(client._player_id));

  if (!joinSuccess) {
    std::cerr << "[ERROR] Client " << client._player_id
              << " failed to join newly created room " << newRoom->getRoomId()
              << std::endl;
    return KO;
  }

  client._state = server::ClientState::IN_ROOM_WAITING;

  if (!server.initializePlayerInRoom(client)) {
    std::cerr << "[ERROR] Failed to initialize player " << client._player_id
              << " in room " << newRoom->getRoomId() << std::endl;
    server.getGameManager().leaveRoom(sharedClient);
    server.getGameManager().destroyRoom(newRoom->getRoomId());
    client._state = server::ClientState::CONNECTED_MENU;
    return KO;
  }

  std::cout << "[CREATE ROOM] Client " << client._player_id
            << " created and joined room " << newRoom->getRoomId() << " ("
            << roomName << ")" << std::endl;

  return OK;
}

int packet::JoinRoomHandler::handlePacket(server::Server &server,
                                          server::Client &client,
                                          const char *data, std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<JoinRoomPacket>(buffer);

  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize JoinRoomPacket from client "
              << client._player_id << std::endl;
    return KO;
  }

  const JoinRoomPacket &packet = deserializedPacket.value();

  auto sharedClient = server.getClientById(client._player_id);
  if (!sharedClient) {
    std::cerr << "[ERROR] Failed to get shared_ptr for client "
              << client._player_id << std::endl;
    return KO;
  }

  auto room = server.getGameManager().getRoom(packet.room_id);
  if (!room) {
    ResponseHelper::sendJoinRoomResponse(server, client._player_id,
                                         RoomError::ROOM_NOT_FOUND);
    return KO;
  }

  if (room->hasPassword()) {
    const size_t passLen = strnlen(packet.password, sizeof(packet.password));
    std::string providedPassword(packet.password, passLen);

    if (!room->checkPassword(providedPassword)) {
      ResponseHelper::sendJoinRoomResponse(server, client._player_id,
                                           RoomError::WRONG_PASSWORD);
      return KO;
    }
  }

  bool joinSuccess =
      server.getGameManager().joinRoom(packet.room_id, sharedClient);

  if (!joinSuccess) {
    std::cout << "[JOIN ROOM] Client " << client._player_id
              << " failed to join room " << packet.room_id << std::endl;

    ResponseHelper::sendJoinRoomResponse(server, client._player_id,
                                         RoomError::ROOM_FULL);
    return KO;
  }

  client._state = server::ClientState::IN_ROOM_WAITING;

  if (!server.initializePlayerInRoom(client)) {
    server.getGameManager().leaveRoom(sharedClient);
    client._state = server::ClientState::CONNECTED_MENU;
    ResponseHelper::sendJoinRoomResponse(server, client._player_id,
                                         RoomError::UNKNOWN_ERROR);
    return KO;
  }

  ResponseHelper::sendJoinRoomResponse(server, client._player_id,
                                       RoomError::SUCCESS);

  return OK;
}

int packet::LeaveRoomHandler::handlePacket(server::Server &server,
                                           server::Client &client,
                                           const char *data, std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<LeaveRoomPacket>(buffer);

  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize LeaveRoomPacket from client "
              << client._player_id << std::endl;
    return KO;
  }

  if (client._room_id == NO_ROOM) {
    std::cout << "[LEAVE ROOM] Client " << client._player_id
              << " is not in any room" << std::endl;
    return OK;
  }

  auto sharedClient = server.getClientById(client._player_id);
  if (!sharedClient) {
    std::cerr << "[ERROR] Failed to get shared_ptr for client "
              << client._player_id << std::endl;
    return KO;
  }

  std::cout << "[LEAVE ROOM] Client " << client._player_id << " leaving room "
            << client._room_id << std::endl;

  server.getGameManager().leaveRoom(sharedClient);

  client._state = server::ClientState::CONNECTED_MENU;

  return OK;
}

int packet::ListRoomHandler::handlePacket(server::Server &server,
                                          server::Client &client,
                                          const char *data, std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<ListRoomPacket>(buffer);

  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize ListRoomPacket from client "
              << client._player_id << std::endl;
    return KO;
  }

  auto rooms = server.getGameManager().getAllRooms();
  std::vector<RoomInfo> roomInfos;
  for (const auto &room : rooms) {
    RoomInfo info;
    info.room_id = room->getRoomId();
    std::strncpy(info.room_name, room->getRoomName().c_str(),
                 sizeof(info.room_name) - 1);
    info.room_name[sizeof(info.room_name) - 1] = '\0';
    info.player_count = room->getPlayerCount();
    info.max_players = room->getMaxPlayers();
    roomInfos.push_back(info);
  }

  auto listRoomResponsePacket = PacketBuilder::makeListRoomResponse(roomInfos);

  auto serializedBuffer =
      serialization::BitserySerializer::serialize(listRoomResponsePacket);
  server.getNetworkManager().sendToClient(
      client._player_id,
      reinterpret_cast<const char *>(serializedBuffer.data()),
      serializedBuffer.size());
  return OK;
}

int packet::MatchmakingRequestHandler::handlePacket(server::Server &server,
                                                    server::Client &client,
                                                    const char *data,
                                                    std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<MatchmakingRequestPacket>(
          buffer);

  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize MatchmakingRequestPacket "
                 "from client "
              << client._player_id << std::endl;
    return KO;
  }

  const MatchmakingRequestPacket &packet = deserializedPacket.value();

  auto sharedClient = server.getClientById(client._player_id);
  if (!sharedClient) {
    ResponseHelper::sendMatchmakingResponse(server, client._player_id,
                                            RoomError::UNKNOWN_ERROR);
    return KO;
  }

  bool joinSuccess = server.getGameManager().joinAnyRoom(sharedClient);

  if (!joinSuccess) {
    auto newRoom = server.getGameManager().createRoom("Matchmaking Room");
    if (!newRoom) {
      std::cerr << "[ERROR] Client " << client._player_id
                << " failed to create new room for matchmaking" << std::endl;
      ResponseHelper::sendMatchmakingResponse(server, client._player_id,
                                              RoomError::UNKNOWN_ERROR);
      return KO;
    }
    auto joinSuccess =
        server.getGameManager().joinRoom(newRoom->getRoomId(), sharedClient);
    if (newRoom && joinSuccess) {
      std::cout << "[MATCHMAKING] Client " << client._player_id
                << " created and joined new room " << newRoom->getRoomId()
                << std::endl;

      client._state = server::ClientState::IN_ROOM_WAITING;

      if (!server.initializePlayerInRoom(client)) {
        server.getGameManager().leaveRoom(sharedClient);
        client._state = server::ClientState::CONNECTED_MENU;
        return KO;
      }

      ResponseHelper::sendMatchmakingResponse(server, client._player_id,
                                              RoomError::SUCCESS);
    } else {
      std::cerr << "[ERROR] Client " << client._player_id
                << " failed to create/join new room for matchmaking"
                << std::endl;
      ResponseHelper::sendMatchmakingResponse(server, client._player_id,
                                              RoomError::UNKNOWN_ERROR);
      return KO;
    }
  } else {
    std::cout << "[MATCHMAKING] Client " << client._player_id
              << " joined existing room" << std::endl;

    client._state = server::ClientState::IN_ROOM_WAITING;

    if (!server.initializePlayerInRoom(client)) {
      std::cerr << "[MATCHMAKING] Failed to initialize player" << std::endl;
      server.getGameManager().leaveRoom(sharedClient);
      client._state = server::ClientState::CONNECTED_MENU;
      return KO;
    }

    ResponseHelper::sendMatchmakingResponse(server, client._player_id,
                                            RoomError::SUCCESS);
  }

  return OK;
}

/**
 * @brief Process a player's input packet, update the player's position, and
 * broadcast the resulting move to the room.
 *
 * Deserializes a PlayerInputPacket from the provided buffer, validates the
 * room and player, updates the player's sequence number and position
 * according to the input and the game's delta time, clamps the position to
 * window bounds, and broadcasts a PlayerMove packet to all clients in the
 * room.
 *
 * @param server Server instance used to access the game manager and network
 * manager.
 * @param client Client that sent the input; used to identify the player and
 * room.
 * @param data Pointer to the raw packet data to deserialize.
 * @param size Size of the raw packet data in bytes.
 * @return int `OK` on success; `KO` on error (for example: deserialization
 * failure, missing or inactive room, or missing player).
 */
int packet::PlayerInputHandler::handlePacket(server::Server &server,
                                             server::Client &client,
                                             const char *data,
                                             std::size_t size) {
  serialization::Buffer buffer(data, data + size);

  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<PlayerInputPacket>(buffer);

  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize PlayerInputPacket from client "
              << client._player_id << std::endl;
    return KO;
  }

  const PlayerInputPacket &packet = deserializedPacket.value();
  auto room = server.getGameManager().getRoom(client._room_id);
  if (!room || !room->isActive()) {
    return KO;
  }

  auto &game = room->getGame();
  float deltaTime = game.getDeltaTime();

  if (deltaTime == 0.0f) {
    deltaTime = 0.016f;
  }

  auto player = room->getGame().getPlayer(client._player_id);
  if (!player) {
    return KO;
  }

  player->setSequenceNumber(packet.sequence_number);

  float moveDistance = player->getSpeed() * deltaTime;

  float dirX = 0.0f;
  float dirY = 0.0f;

  if (packet.input & static_cast<std::uint8_t>(MovementInputType::UP))
    dirY -= 1.0f;
  if (packet.input & static_cast<std::uint8_t>(MovementInputType::DOWN))
    dirY += 1.0f;
  if (packet.input & static_cast<std::uint8_t>(MovementInputType::LEFT))
    dirX -= 1.0f;
  if (packet.input & static_cast<std::uint8_t>(MovementInputType::RIGHT))
    dirX += 1.0f;

  float length = std::sqrt(dirX * dirX + dirY * dirY);
  if (length > 0.0f) {
    dirX /= length;
    dirY /= length;
  }

  float newX = player->getPosition().first + dirX * moveDistance;
  float newY = player->getPosition().second + dirY * moveDistance;

  newX =
      std::clamp(newX, 0.0f, static_cast<float>(WINDOW_WIDTH) - PLAYER_WIDTH);
  newY =
      std::clamp(newY, 0.0f, static_cast<float>(WINDOW_HEIGHT) - PLAYER_HEIGHT);

  player->setPosition(newX, newY);

  auto movePacket = PacketBuilder::makePlayerMove(
      client._player_id, player->getSequenceNumber().value_or(0), newX, newY);

  auto roomClients = room->getClients();
  broadcast::Broadcast::broadcastPlayerMoveToRoom(server.getNetworkManager(),
                                                  roomClients, movePacket);

  return OK;
}
