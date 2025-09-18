#include "Parser.hpp"
#include "Errors/ParamsError.hpp"
#include "Macros.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

int Parser::parseServerProperties() {
  std::ifstream ifs(SERVER_PROPERTIES);
  if (!ifs.is_open()) {
    std::cout << "No server.properties file found, using default values."
              << std::endl;
    return SUCCESS;
  }
  std::string line;
  while (std::getline(ifs, line)) {
    if (line.find("PORT") == 0) {
      std::string port = line.substr(line.find('=') + 1);
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
    } else if (line.find("MAX_PLAYER") == 0) {
      std::string max_player = line.substr(line.find('=') + 1);
      if (!max_player.empty())
        try {
          _max_player = std::stoi(max_player);
        } catch (const std::invalid_argument &e) {
          throw ParamsError("Invalid max player in server properties file.");
        } catch (const std::out_of_range &e) {
          throw ParamsError("Max player value out of range.");
        }
      else {
        throw ParamsError("Invalid max player in server properties file.");
      }
    }
  }
  ifs.close();
  return SUCCESS;
}
