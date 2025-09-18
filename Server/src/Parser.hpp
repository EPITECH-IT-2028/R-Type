#pragma once

class Parser {
public:
  Parser() = default;
  ~Parser() = default;

  int parseServerProperties();
  int getPort() const { return _port; }
  int getMaxPlayer() const { return _max_player; }

private:
  int _port = 4242;
  int _max_player = 4;
};
