#pragma once

#include <cstdint>
class Parser {
  public:
    Parser() = default;
    ~Parser() = default;

    void parseServerProperties();
    std::uint16_t getPort() const {
      return _port;
    }
    std::uint16_t getMaxClients() const {
      return _max_clients;
    }

  private:
    std::uint16_t _port = 4242;
    std::uint16_t _max_clients = 4;
};
