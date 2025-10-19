#include "PacketHandler.hpp"
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iostream>
#include "Broadcast.hpp"
#include "GameManager.hpp"
#include "Macro.hpp"
#include "Packet.hpp"
#include "PacketSerialize.hpp"
#include "Server.hpp"

void packet::ResponseHelper::sendJoinRoomResponse(server::Server &server,
                                                  uint32_t player_id,
                                                  RoomError error) {
  auto response = PacketBuilder::makeJoinRoomResponse(error);
  auto serializedBuffer = serialization::BitserySerializer::serialize(response);
  server.getNetworkManager().sendToClient(
      player_id, reinterpret_cast<const char *>(serializedBuffer.data()),
      serializedBuffer.size());
}

void packet::ResponseHelper::sendMatchmakingResponse(server::Server &server,
                                                     uint32_t player_id,
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
  validatedPacket.player_id = static_cast<uint32_t>(client._player_id);

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

int packet::PositionHandler::handlePacket(server::Server &server,
                                          server::Client &client,
                                          const char *data, std::size_t size) {
  serialization::Buffer buffer(data, data + size);
  auto deserializedPacket =
      serialization::BitserySerializer::deserialize<PositionPacket>(buffer);
  if (!deserializedPacket) {
    std::cerr << "[ERROR] Failed to deserialize PositionPacket from client "
              << client._player_id << std::endl;
    return KO;
  }
  const PositionPacket &packet = deserializedPacket.value();

  auto room = server.getGameManager().getRoom(client._room_id);
  if (!room)
    return KO;

  auto player = room->getGame().getPlayer(client._player_id);
  if (!player) {
    return KO;
  }

  float oldX = player->getPosition().first;
  float oldY = player->getPosition().second;

  auto now = std::chrono::steady_clock::now();
  auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(
      now - client._last_position_update);

  if (timeDiff.count() > 5) {
    float deltaX = packet.x - oldX;
    float deltaY = packet.y - oldY;
    float distance = std::sqrt(deltaX * deltaX + deltaY * deltaY);

    float timeSeconds = timeDiff.count() / 1000.0f;
    float actualSpeed = distance / timeSeconds;
    float maxSpeed = (player->getSpeed() * FPS) * TOLERANCE;

    if (actualSpeed > maxSpeed) {
      std::cout << "[ANTICHEAT] Player " << client._player_id
                << " is moving too fast!" << std::endl;
      player->setSequenceNumber(packet.sequence_number);
      return KO;
    }
  }

  player->setPosition(packet.x, packet.y);
  player->setSequenceNumber(packet.sequence_number);
  client._last_position_update = now;
  std::pair<float, float> pos = player->getPosition();

  auto movePacket = PacketBuilder::makeMove(
      client._player_id, player->getSequenceNumber().value_or(0), pos.first,
      pos.second);

  auto roomClients = room->getClients();
  broadcast::Broadcast::broadcastPlayerMoveToRoom(server.getNetworkManager(),
                                                  roomClients, movePacket);
  return OK;
}

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
  if (hb.player_id != static_cast<uint32_t>(client._player_id)) {
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
  if (disc.player_id != static_cast<uint32_t>(client._player_id)) {
    return KO;
  }
  std::cout << "[WORLD] Player " << client._player_id << " disconnected."
            << std::endl;

  bool wasConnected = client._connected;
  client._connected = false;

  if (wasConnected) {
    server.setPlayerCount(server.getPlayerCount() - 1);
  }

  if (client._room_id != -1) {
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

  if (joinSuccess) {
    std::cout << "[JOIN ROOM] Client " << client._player_id
              << " successfully joined room " << packet.room_id << std::endl;
  } else {
    std::cout << "[JOIN ROOM] Client " << client._player_id
              << " failed to join room " << packet.room_id << std::endl;

    ResponseHelper::sendJoinRoomResponse(server, client._player_id,
                                         RoomError::ROOM_FULL);
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
    if (newRoom && newRoom->addClient(sharedClient)) {
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
