#pragma once

#include <sqlite3.h>
#include <string>
#include <vector>

namespace database {

  struct PlayerData {
      int id;
      std::string username;
      std::string ip_address;
      bool is_online;
  };

  struct BanData {
      int id;
      std::string ip_address;
      std::string reason;
  };

  class DatabaseManager {
    public:
      DatabaseManager(const std::string &dbPath = "./rtype.db");
      ~DatabaseManager();

      DatabaseManager(const DatabaseManager &) = delete;
      DatabaseManager &operator=(const DatabaseManager &) = delete;

      bool initialize();
      void close();

      bool addPlayer(const std::string &username, const std::string &ipAddress);
      bool removePlayer(int playerId);
      bool updatePlayerStatus(int playerId, bool isOnline);
      std::optional<PlayerData> getPlayerByUsername(
          const std::string &username);
      std::optional<PlayerData> getPlayerByIP(const std::string &ip_address);
      bool isIpBanned(const std::string &ip_address);
      std::vector<PlayerData> getAllPlayers();
      std::vector<BanData> getAllBans();

    private:
      bool executeQuery(const std::string &query);

      std::vector<PlayerData> getResultQuery(const std::string &query);

      sqlite3 *_db;
      std::string _dbPath;
  };

}  // namespace database
