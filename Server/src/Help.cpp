#include "Help.hpp"
#include <iostream>
#include "Macros.hpp"

int help() {
  std::cout << "Usage: ./r-type_server PORT" << std::endl;
  std::cout << "  PARAMS: " << std::endl;
  std::cout << "\tPORT: a port to connect between 1 and 65535" << std::endl;
  std::cout << "\t-help: Show this help message" << std::endl;
  return SUCCESS;
}

