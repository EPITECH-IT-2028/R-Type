#pragma once

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace crypto {

  class Crypto {
    public:
      static std::string sha256(const std::string &input) {
        unsigned char hash[SHA256_DIGEST_LENGTH];

        SHA256(reinterpret_cast<const unsigned char *>(input.c_str()),
               input.length(), hash);

        std::stringstream ss;
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
          ss << std::hex << std::setw(2) << std::setfill('0')
             << static_cast<int>(hash[i]);
        }
        return ss.str();
      }

      static std::string generateChallenge(size_t length = 64) {
        std::vector<unsigned char> buffer(length);
        if (RAND_bytes(buffer.data(), length) != 1) {
          throw std::runtime_error("RAND_bytes failed");
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
