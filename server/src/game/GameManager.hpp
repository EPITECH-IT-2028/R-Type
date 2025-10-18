#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include "GameRoom.hpp"

namespace server {
  struct Client;
}

namespace game {

  class GameManager {
    public:
      GameManager(int maxPlayers = 4);
      ~GameManager();

      std::shared_ptr<GameRoom> createRoom(const std::string &roomName = "",
                                           const std::string &password = "");
      bool destroyRoom(int roomId);
      std::shared_ptr<GameRoom> findAvailableRoom();
      std::shared_ptr<GameRoom> getRoom(uint32_t roomId) const;
      bool joinRoom(int roomId, std::shared_ptr<server::Client> client);
      bool joinAnyRoom(std::shared_ptr<server::Client> client);
      void leaveRoom(std::shared_ptr<server::Client> client);
      std::vector<std::shared_ptr<GameRoom>> getAllRooms() const;
      void removeEmptyRooms();
      size_t getRoomCount() const;
      void shutdownRooms();

    private:
      std::unordered_map<uint32_t, std::shared_ptr<game::GameRoom>> _rooms;
      int _maxPlayers;
      std::atomic<uint32_t> _nextRoomId;
      mutable std::mutex _roomMutex;
  };
}  // namespace game
