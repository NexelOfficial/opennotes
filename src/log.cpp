#include "log.hpp"

#include <iostream>
#include <string>

auto Log::error(STATUS code, std::string prefix) -> STATUS {
  std::string err_buffer = "";
  err_buffer.resize(MAXWORD);
  OSLoadString(nullptr, code, err_buffer.data(), MAXWORD);

  if (err_buffer.empty()) {
    err_buffer += std::to_string(code);
  }

  std::cout << prefix << ": " << err_buffer.data() << "\n";
  return code;
}

void Log::error(std::string message) { std::cout << message << "\n"; }