#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <stdexcept>
#include "ParamsError.hpp"

/* Macros for files paths */
constexpr const char *SERVER_PROPERTIES = "server/server.properties";
constexpr const char *CLIENT_PROPERTIES = "client/client.properties";

/* Macros for ports */
constexpr int MAX_PORT = 65535;
constexpr int MIN_PORT = 1;

class Parser {
  public:
    Parser(std::string propertiesPath) : _propertiesPath(std::move(propertiesPath)) {}
    ~Parser() = default;

    void parseProperties();

    bool isValidIp(const std::string &ip) const;
    
    std::string trimString(const std::string &str) const;

    std::uint16_t getPort() const {
      return _port;
    }
    std::uint16_t getMaxClients() const {
      return _max_clients;
    }

    std::string getIp() const {
      return _ip;
    }

  private:
    const std::string _propertiesPath;
    std::uint16_t _port = 4242;
    std::string _ip = "127.0.0.1";
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
            {"IP",
             [this](const std::string &ip) {
               if (!ip.empty()) {
                 _ip = ip;
               } else {
                 throw ParamsError("Invalid IP in client properties file.");
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
