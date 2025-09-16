#include <asio.hpp>
#include <iostream>
#include "Parser.hpp"
#include "Errors/ParamsError.hpp"
#include "Help.hpp"
#include "Server.hpp"
#include "Macros.hpp"

int main(int ac, char **av) {
  try {
    if (ac != 2 || !av) {
      throw ParamsError("Not enough arguments, check -help for more informations.");
    }
    if (strcmp(av[1], "-help") == 0) {
      return help();
    }
    int port = Parser::isValidPort(av[1]);
    asio::io_context io_context;

    server::Server server(io_context, port);

    std::cout << "Starting server on port " << port << "..." << std::endl;
    server.start();

    asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait(
        [&server](const asio::error_code &error, __attribute__((unused)) int) {
          if (!error) {
            std::cout << "\nStopping server..." << std::endl;
            server.stop();
          }
        });
    io_context.run();
  } catch (ParamsError &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return ERROR;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return ERROR;
  }
  return SUCCESS;
}
