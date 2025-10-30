#pragma once
#include <domino/global.h>

#include <string>

#include "view.hpp"

class Database {
 public:
  Database(std::string server, std::string file) : Database(std::string{}, server, file) {}
  Database(std::string port, std::string server, std::string file);
  Database(Database& other) = delete;
  ~Database();

  [[nodiscard]] auto get_view(std::string name) const -> View { return {this->handle, name}; }

  [[nodiscard]] auto is_valid() const -> bool { return this->handle != NULLHANDLE; }
  [[nodiscard]] auto get_handle() const -> DHANDLE { return this->handle; }

 private:
  DHANDLE handle = NULLHANDLE;
};