#include "Parser.hpp"
#include <fstream>
#include <iostream>
#include "Errors/ParamsError.hpp"
#include "Macros.hpp"

void Parser::parseServerProperties() {
  std::ifstream ifs(SERVER_PROPERTIES);
  if (!ifs.is_open()) {
    std::cerr << "No " << SERVER_PROPERTIES
              << " file found, using default values.";
    return;
  }
  std::string line;
  while (std::getline(ifs, line)) {
    if (line.empty() || line[0] == '#')
      continue;
    auto pos = line.find('=');
    if (pos == std::string::npos)
      continue;
    std::string key = line.substr(0, pos);
    std::string value = line.substr(pos + 1);
    std::cout << "Key: " << key << ", Value: " << value << std::endl;
    auto it = _propertyParsers.find(key);
    if (it != _propertyParsers.end()) {
      it->second(value);
    }
  }
  if (_max_clients <= 0 || _port <= MIN_PORT || _port > MAX_PORT) {
    throw ParamsError("Invalid server properties.");
  }
  return;
}
