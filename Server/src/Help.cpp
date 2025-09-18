#include "Help.hpp"
#include <iostream>
#include "Macros.hpp"

namespace Help {
  int help() {
    std::cout << "Usage: ./r-type_server" << std::endl;
    std::cout << "\t-help: Show this help message" << std::endl;
    return SUCCESS;
  }
}

