#include "Parser.hpp"
#include "Errors/ParamsError.hpp"
#include "Macros.hpp"
#include <exception>

namespace Parser {
  int isValidPort(const std::string& port) {
    int validPort = 0;

    if (port.empty())
      throw ParamsError("Port is empty.");
    if (std::stoi(port) < MIN_PORT || std::stoi(port) > MAX_PORT)
      throw ParamsError("Invalid port. Port must be between 0 and 65535.");
    try {
      validPort = std::stoi(port);
      return validPort;
    } catch (const std::exception &e) {
      throw ParamsError("Port is too high or too low. The value of port must be between 0 and 65535.");
    }
  }
}
