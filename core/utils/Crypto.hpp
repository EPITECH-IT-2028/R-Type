#pragma once

#include <picosha2.h>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace crypto {

  class Crypto {
    public:
      static std::string sha256(const std::string &input) {
        std::string hash_hex;
        picosha2::hash256_hex_string(input, hash_hex);
        return hash_hex;
      }

      /**
       * @brief Generates a random hexadecimal challenge string.
       *
       * Generates a random sequence of bytes of the specified length,
       * then encodes it as a hexadecimal string.
       * @param length Length of the challenge in bytes (default is 64).
       * @return std::string Hexadecimal representation of the random challenge.
       */
      static std::string generateChallenge(size_t length) {
        thread_local std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<unsigned int> dis(0, 255);

        std::vector<unsigned char> buffer(length);
        for (size_t i = 0; i < length; i++) {
          buffer[i] = static_cast<unsigned char>(dis(gen));
        }

        std::stringstream ss;
        for (unsigned char byte : buffer) {
          ss << std::hex << std::setw(2) << std::setfill('0')
             << static_cast<int>(byte);
        }
        return ss.str();
      }
  };

}  // namespace crypto
