#include <asio.hpp>
#include <iostream>
#include "Server.hpp"

int main() {
  try {
    int port = 7777;
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

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
