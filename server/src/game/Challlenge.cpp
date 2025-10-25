#include <cstdint>
#include <iostream>
#include "Challenge.hpp"
#include "crypto/Crypto.hpp"

std::uint32_t game::Challenge::getCurrentTimestamp() const {
  auto now = std::chrono::system_clock::now();
  auto seconds =
      std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
  return static_cast<std::uint32_t>(seconds.count());
}

std::string game::Challenge::createChallenge(std::uint32_t playerId) {
  std::lock_guard<std::mutex> lock(_mutex);

  std::string nonce = crypto::Crypto::generateChallenge(32);
  std::uint32_t timestamp = getCurrentTimestamp();

  _challenges[playerId] = {nonce, timestamp};

  return nonce;
}

bool game::Challenge::validateJoinRoom(std::uint32_t player_id,
                                       const std::string &provided_hash,
                                       const std::string &original_password) {
  std::lock_guard<std::mutex> lock(_mutex);

  auto it = _challenges.find(player_id);
  if (it == _challenges.end()) {
    return false;
  }

  std::uint32_t current_time = getCurrentTimestamp();
  if (current_time - it->second.timestamp > CHALLENGE_TIMEOUT) {
    _challenges.erase(it);
    return false;
  }

  std::string expected_hash =
      crypto::Crypto::sha256(it->second.nonce + original_password);

  std::cout << "Expected Hash: " << expected_hash << std::endl;
  std::cout << "Provided Hash: " << provided_hash << std::endl;

  bool is_valid = (expected_hash == provided_hash);
  _challenges.erase(it);
  return is_valid;
}
