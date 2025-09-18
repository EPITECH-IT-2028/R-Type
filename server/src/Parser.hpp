#pragma once

class Parser {
public:
  Parser() = default;
  ~Parser() = default;

  int parseServerProperties();
  int getPort() const { return _port; }
  int getMaxClients() const { return _max_clients; }

private:
  int _port = 4242;
  int _max_clients = 4;
};