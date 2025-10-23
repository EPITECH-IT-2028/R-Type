#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #ifndef _WIN32_WINNT
        #define _WIN32_WINNT 0x0601
    #endif
#endif

#include <asio.hpp>
#include <csignal>
#include <cstring>
#include <iostream>
#include "Help.hpp"
#include "Macro.hpp"
#include "ParamsError.hpp"
#include "Parser.hpp"
#include "Server.hpp"

int main(int ac, char **av) {
  try {
    if (ac == 2 && std::strcmp(av[1], "--help") == 0) {
      Help::help();
      return OK;
    } else if (ac >= 2) {
      throw ParamsError(
          "Too much arguments, check --help for more informations.");
    }

    Parser parser(SERVER_PROPERTIES);
    parser.parseProperties();

    server::Server server(parser.getPort(), parser.getMaxClients(),
                          parser.getClientsPerRoom());

    std::cout << "Starting server on port " << parser.getPort() << "..."
              << std::endl;
    server.start();

  } catch (const ParamsError &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return KO;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return KO;
  }
  return OK;
}
