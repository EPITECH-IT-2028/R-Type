#include "Parser.hpp"
#include "Errors/ParamsError.hpp"
#include "Macros.hpp"
#include <fstream>
#include <iostream>

void Parser::parseServerProperties() {
  std::ifstream ifs(SERVER_PROPERTIES);
  if (!ifs.is_open()) {
    std::cerr << "No " << SERVER_PROPERTIES
              << " file found, using default values.";
    return;
  }
  std::string line;
  while (std::getline(ifs, line)) {
    if (line.find("PORT=") == 0) {
      if (line.empty() || line.at(0) == '#')
        continue;
      std::string port = line.substr(PORT_LENGTH);
      if (!port.empty()) {
        try {
          _port = std::stoi(port);
        } catch (const std::invalid_argument &e) {
          throw ParamsError("Invalid port in server properties file.");
        } catch (const std::out_of_range &e) {
          throw ParamsError("Port value out of range.");
        }
      } else {
        throw ParamsError("Invalid port in server properties file.");
      }
    } else if (line.find("MAX_CLIENTS=") == 0) {
      std::string max_clients = line.substr(MAX_CLIENTS_LENGTH);
      if (!max_clients.empty())
        try {
          _max_clients = std::stoi(max_clients);
        } catch (const std::invalid_argument &e) {
          throw ParamsError("Invalid max clients in server properties file.");
        } catch (const std::out_of_range &e) {
          throw ParamsError("Max clients value out of range.");
        }
      else {
        throw ParamsError("Invalid max clients in server properties file.");
      }
    }
  }
  if (_max_clients <= 0 || _port <= MIN_PORT || _port > MAX_PORT) {
    throw ParamsError("Invalid server properties.");
  }
  return;
}
