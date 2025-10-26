#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <unordered_map>
namespace game {

  class Challenge {
      struct ChallengeStruct {
          std::string nonce;
          std::uint32_t timestamp;
      };

    public:
      Challenge() = default;
      Challenge(const Challenge &) = delete;
      Challenge &operator=(const Challenge &) = delete;
      Challenge(Challenge &&) = delete;
      Challenge &operator=(Challenge &&) = delete;

      std::string createChallenge(std::uint32_t playerId);

      bool validateJoinRoom(std::uint32_t player_id,
                            const std::string &provided_hash,
                            const std::string &original_password);

    private:
      std::unordered_map<std::uint32_t, ChallengeStruct> _challenges;
      std::mutex _mutex;

      static constexpr std::uint32_t CHALLENGE_TIMEOUT = 30;

      std::uint32_t getCurrentTimestamp() const;
  };

}  // namespace game
