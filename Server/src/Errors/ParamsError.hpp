#pragma once

#include <exception>
#include <string>

class ParamsError : public std::exception {
public:
  ParamsError(std::string msg) : _msg(msg) {}

  const char *what() const noexcept override { return _msg.c_str(); }


private:
  std::string _msg;
};