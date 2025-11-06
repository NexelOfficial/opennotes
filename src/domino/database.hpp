#pragma once
#include <domino/global.h>

#include <optional>
#include <string>

#include "view.hpp"

class Database {
 public:
  Database(std::string server, std::string file) : Database(std::string{}, server, file) {}
  Database(std::string server, std::string file, std::optional<std::string> port);
  Database(Database& other) = delete;
  ~Database();

  [[nodiscard]] auto get_view(const std::string& name) const -> View {
    return {this->handle, name};
  }

  [[nodiscard]] auto is_valid() const -> bool { return this->handle != NULLHANDLE; }
  [[nodiscard]] auto get_handle() const -> DHANDLE { return this->handle; }

 private:
  DHANDLE handle = NULLHANDLE;
};