#include "Parser.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include "ParamsError.hpp"

std::string Parser::trimString(const std::string &str) const {
  size_t first = str.find_first_not_of(" \t");
  if (first == std::string::npos)
    return "";
  size_t last = str.find_last_not_of(" \t\r");
  return str.substr(first, last - first + 1);
}

void Parser::parseProperties() {
  if (_propertiesPath != SERVER_PROPERTIES && _propertiesPath != CLIENT_PROPERTIES) {
    throw ParamsError("Invalid properties file path.");
  }

  std::ifstream ifs(_propertiesPath);
  if (!ifs.is_open()) {
    std::cerr << "No " << _propertiesPath
              << " file found, using default values.";
    return;
  }

  std::string line;
  while (std::getline(ifs, line)) {
    auto first = line.find_first_not_of(" \t");
    if (first == std::string::npos || line[first] == '#')
      continue;
    auto pos = line.find('=', first);
    if (pos == std::string::npos)
      continue;
    std::string key = line.substr(first, pos - first);
    std::string value = line.substr(pos + 1);
    key = trimString(key);
    value = trimString(value);
    std::transform(key.begin(), key.end(), key.begin(), ::toupper);
    auto it = _propertyParsers.find(key);
    if (it != _propertyParsers.end()) {
      it->second(value);
    } else {
      std::cerr << "Unknown property: " << key << std::endl;
    }
  }

  if (_propertiesPath == SERVER_PROPERTIES) {
    if (_max_clients <= 0 || _port <= MIN_PORT || _port > MAX_PORT) {
      throw ParamsError("Invalid server properties.");
    }
  } else if (_propertiesPath == CLIENT_PROPERTIES) {
    if (_port <= MIN_PORT || _port > MAX_PORT) {
      throw ParamsError("Invalid client properties.");
    }
  }
}

// void Parser::parseServerProperties() {
//   std::ifstream ifs(_propertiesPath);
//   if (!ifs.is_open()) {
//     std::cerr << "No " << _propertiesPath
//               << " file found, using default values.";
//     return;
//   }
//   std::string line;
//   while (std::getline(ifs, line)) {
//     auto first = line.find_first_not_of(" \t");
//     if (first == std::string::npos || line[first] == '#')
//       continue;
//     auto pos = line.find('=', first);
//     if (pos == std::string::npos)
//       continue;
//     std::string key = line.substr(first, pos - first);
//     std::string value = line.substr(pos + 1);
//     key = trimString(key);
//     value = trimString(value);
//     std::transform(key.begin(), key.end(), key.begin(), ::toupper);
//     auto it = _propertyParsers.find(key);
//     if (it != _propertyParsers.end()) {
//       it->second(value);
//     } else {
//       std::cerr << "Unknown property: " << key << std::endl;
//     }
//   }
//   if (_max_clients <= 0 || _port <= MIN_PORT || _port > MAX_PORT) {
//     throw ParamsError("Invalid server properties.");
//   }
//   return;
// }
