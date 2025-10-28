#include "DatabaseManager.hpp"
#include <sqlite3.h>
#include <fstream>
#include <iostream>

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
    row.username = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
    row.ip_address =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
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

  std::ifstream sqlFile("db.sql");
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

  std::string query =
      "INSERT INTO players (username, ip_address, is_online) "
      "VALUES ('" +
      username + "', '" + ipAddress + "', 0);";
  return executeQuery(query);
}

bool database::DatabaseManager::removePlayer(int playerId) {
  std::string query =
      "DELETE FROM players WHERE id = " + std::to_string(playerId) + ";";
  return executeQuery(query);
}

bool database::DatabaseManager::updatePlayerStatus(int playerId,
                                                   bool isOnline) {
  std::string query =
      "UPDATE players SET is_online = " + std::to_string(isOnline ? 1 : 0) +
      " WHERE id = " + std::to_string(playerId) + ";";
  return executeQuery(query);
}

std::optional<database::PlayerData>
database::DatabaseManager::getPlayerByUsername(const std::string &username) {
  const std::string query =
      "SELECT id, username, ip_address, is_online FROM players WHERE username "
      "= '" +
      username + "';";

  auto results = getResultQuery(query);
  if (results.empty()) {
    return std::nullopt;
  }
  return results.front();
}

std::optional<database::PlayerData> database::DatabaseManager::getPlayerByIP(
    const std::string &ip_address) {
  const std::string query =
      "SELECT id, username, ip_address, is_online FROM players WHERE "
      "ip_address "
      "= '" +
      ip_address + "';";
  auto results = getResultQuery(query);
  if (results.empty()) {
    return std::nullopt;
  }
  return results.front();
}

std::vector<database::PlayerData> database::DatabaseManager::getAllPlayers() {
  const std::string query =
      "SELECT id, username, ip_address, is_online FROM players;";
  return getResultQuery(query);
}

bool database::DatabaseManager::isIpBanned(const std::string &ip_address) {
  const std::string query =
      "SELECT COUNT(*) FROM bans WHERE ip_address = '" + ip_address + "';";
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(_db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
    return false;
  }

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
    row.ip_address =
        reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
    row.reason = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
    bans.push_back(row);
  }

  sqlite3_finalize(stmt);
  return bans;
}
