#include "DatabaseManager.hpp"
#include <sqlite3.h>
#include <fstream>
#include <iostream>
#include "Macro.hpp"

database::DatabaseManager::DatabaseManager(const std::string &dbPath)
    : _db(nullptr), _dbPath(dbPath) {
}

database::DatabaseManager::~DatabaseManager() {
  close();
}

bool database::DatabaseManager::executeQuery(const std::string &query) {
  char *errMsg = nullptr;
  if (sqlite3_exec(_db, query.c_str(), nullptr, nullptr, &errMsg) !=
      SQLITE_OK) {
    std::cerr << "[DATABASE ERROR] SQL error: " << errMsg << std::endl;
    sqlite3_free(errMsg);
    return false;
  }
  return true;
}

std::vector<database::PlayerData> database::DatabaseManager::getResultQuery(
    const std::string &query) {
  std::vector<PlayerData> data;
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(_db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    return data;
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    PlayerData row;
    row.id = sqlite3_column_int(stmt, 0);
    const char *username_text =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
    const char *ip_text =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
    row.username = username_text ? username_text : "";
    row.ip_address = ip_text ? ip_text : "";
    row.is_online = sqlite3_column_int(stmt, 3) != 0;
    data.push_back(row);
  }

  sqlite3_finalize(stmt);
  return data;
}

bool database::DatabaseManager::initialize() {
  if (sqlite3_open(_dbPath.c_str(), &_db) != SQLITE_OK) {
    std::cerr << "[DATABASE ERROR] Failed to open database: "
              << sqlite3_errmsg(_db) << std::endl;
    return false;
  }

  std::ifstream sqlFile(SQL_PATH);
  if (!sqlFile.is_open()) {
    std::cerr << "[DATABASE ERROR] Failed to open db.sql file." << std::endl;
    return false;
  }

  std::string sqlQuery((std::istreambuf_iterator<char>(sqlFile)),
                       std::istreambuf_iterator<char>());
  sqlFile.close();

  if (!executeQuery(sqlQuery)) {
    std::cerr << "[DATABASE ERROR] Failed to execute initialization SQL."
              << std::endl;
    return false;
  }

  return true;
}

void database::DatabaseManager::close() {
  if (_db) {
    sqlite3_close(_db);
    _db = nullptr;
  }
}

bool database::DatabaseManager::addPlayer(const std::string &username,
                                          const std::string &ipAddress) {
  if (username.empty() || ipAddress.empty()) {
    return false;
  }
  if (getPlayerByUsername(username).has_value()) {
    return false;
  }

  const char *query =
      "INSERT INTO players (username, ip_address, is_online) VALUES (?, ?, 0)";
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(_db, query, -1, &stmt, nullptr) != SQLITE_OK) {
    return false;
  }
  sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 2, ipAddress.c_str(), -1, SQLITE_TRANSIENT);
  bool success = (sqlite3_step(stmt) == SQLITE_DONE);
  sqlite3_finalize(stmt);
  return success;
}

bool database::DatabaseManager::removePlayer(int playerId) {
  const char *query = "DELETE FROM players WHERE id = ?";
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(_db, query, -1, &stmt, nullptr) != SQLITE_OK) {
    return false;
  }
  sqlite3_bind_int(stmt, 1, playerId);
  bool success = (sqlite3_step(stmt) == SQLITE_DONE);
  sqlite3_finalize(stmt);
  return success;
}

bool database::DatabaseManager::updatePlayerStatus(const std::string &username,
                                                   bool isOnline) {
  const char *query = "UPDATE players SET is_online = ? WHERE username = ?";
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(_db, query, -1, &stmt, nullptr) != SQLITE_OK) {
    return false;
  }
  sqlite3_bind_int(stmt, 1, isOnline ? 1 : 0);
  sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_TRANSIENT);
  bool success = (sqlite3_step(stmt) == SQLITE_DONE);
  sqlite3_finalize(stmt);
  return success;
}

std::optional<database::PlayerData>
database::DatabaseManager::getPlayerByUsername(const std::string &username) {
  const char *query =
      "SELECT id, username, ip_address, is_online FROM players WHERE username "
      "= ?";
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(_db, query, -1, &stmt, nullptr) != SQLITE_OK) {
    return std::nullopt;
  }
  sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

  std::optional<PlayerData> result;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    PlayerData row;
    row.id = sqlite3_column_int(stmt, 0);
    const char *username_text =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
    const char *ip_text =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
    row.username = username_text ? username_text : "";
    row.ip_address = ip_text ? ip_text : "";
    row.is_online = sqlite3_column_int(stmt, 3) != 0;
    result = row;
  }
  sqlite3_finalize(stmt);
  return result;
}

std::optional<database::PlayerData> database::DatabaseManager::getPlayerByIP(
    const std::string &ip_address) {
  const std::string query =
      "SELECT id, username, ip_address, is_online FROM players WHERE "
      "ip_address "
      "= ?";
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(_db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    return std::nullopt;
  }
  sqlite3_bind_text(stmt, 1, ip_address.c_str(), -1, SQLITE_TRANSIENT);
  std::optional<PlayerData> result;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    PlayerData row;
    row.id = sqlite3_column_int(stmt, 0);
    const char *username_text =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
    const char *ip_text =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
    row.username = username_text ? username_text : "";
    row.ip_address = ip_text ? ip_text : "";
    row.is_online = sqlite3_column_int(stmt, 3) != 0;
    result = row;
  }
  sqlite3_finalize(stmt);
  return result;
}

std::vector<database::PlayerData> database::DatabaseManager::getAllPlayers() {
  const std::string query =
      "SELECT id, username, ip_address, is_online FROM players;";
  return getResultQuery(query);
}

bool database::DatabaseManager::isIpBanned(const std::string &ip_address) {
  const char *query = "SELECT COUNT(*) FROM bans WHERE ip_address = ?";
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(_db, query, -1, &stmt, nullptr) != SQLITE_OK) {
    return false;
  }

  sqlite3_bind_text(stmt, 1, ip_address.c_str(), -1, SQLITE_TRANSIENT);

  bool isBanned = false;
  if (sqlite3_step(stmt) == SQLITE_ROW) {
    int count = sqlite3_column_int(stmt, 0);
    isBanned = (count > 0);
  }
  sqlite3_finalize(stmt);
  return isBanned;
}

std::vector<database::BanData> database::DatabaseManager::getAllBans() {
  std::vector<BanData> bans;
  sqlite3_stmt *stmt;
  const std::string query = "SELECT id, ip_address, reason FROM bans;";

  if (sqlite3_prepare_v2(_db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    return bans;
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    BanData row;
    row.id = sqlite3_column_int(stmt, 0);
    const char *ip_text =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
    const char *reason_text =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
    row.ip_address = ip_text ? ip_text : "";
    row.reason = reason_text ? reason_text : "";
    bans.push_back(row);
  }

  sqlite3_finalize(stmt);
  return bans;
}
