#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include "ParamsError.hpp"

class Parser {
  public:
    Parser() = default;
    ~Parser() = default;

    void parseServerProperties();
    std::string trimString(const std::string &str) const;

    std::uint16_t getPort() const {
      return _port;
    }
    std::uint16_t getMaxClients() const {
      return _max_clients;
    }

  private:
    std::uint16_t _port = 4242;
    std::uint16_t _max_clients = 4;
    std::unordered_map<std::string, std::function<void(const std::string &)>>
        _propertyParsers = {
            {"PORT",
             [this](const std::string &port) {
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
             }},
            {"MAX_CLIENTS",
             [this](const std::string &max_clients) {
               if (!max_clients.empty()) {
                 try {
                   _max_clients = std::stoi(max_clients);
                 } catch (const std::invalid_argument &e) {
                   throw ParamsError(
                       "Invalid max clients in server properties file.");
                 } catch (const std::out_of_range &e) {
                   throw ParamsError("Max clients value out of range.");
                 }
               } else {
                 throw ParamsError(
                     "Invalid max clients in server properties file.");
               }
             }},
    };
};
