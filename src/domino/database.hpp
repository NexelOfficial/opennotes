#pragma once
#include <domino/global.h>

#include <string>

class Database {
 public:
  Database(std::string server, std::string file) : Database(std::string{}, server, file) {}
  Database(std::string port, std::string server, std::string file);
  Database(Database& other) = delete;
  ~Database();

  [[nodiscard]] auto is_valid() const -> bool { return this->handle != NULLHANDLE; }
  [[nodiscard]] auto get_handle() const -> DHANDLE { return this->handle; }

 private:
  DHANDLE handle = NULLHANDLE;
};