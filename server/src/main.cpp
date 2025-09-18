#include "Errors/ParamsError.hpp"
#include "Help.hpp"
#include "Macros.hpp"
#include "Parser.hpp"
#include "Server.hpp"
#include <asio.hpp>
#include <iostream>

int main(int ac, char **av) {
  try {
    if (!av) {
      throw ParamsError(
          "Not enough arguments, check -help for more informations.");
    }
    if (ac == 2 && strcmp(av[1], "-help") == 0) {
      Help::help();
      return SUCCESS;
    } else if (ac >= 2) {
      throw ParamsError(
          "Too much arguments, check -help for more informations.");
    }

    Parser parser;
    parser.parseServerProperties();

    asio::io_context io_context;

    server::Server server(io_context, parser.getPort(), parser.getMaxClients());

    std::cout << "Starting server on port " << parser.getPort() << "..."
              << std::endl;
    server.start();

    asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait(
        [&server](const asio::error_code &error, [[maybe_unused]] int) {
          if (!error) {
            std::cout << "\nStopping server..." << std::endl;
            server.stop();
          }
        });
    io_context.run();
  } catch (const ParamsError &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return ERROR;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return ERROR;
  }
  return SUCCESS;
}
