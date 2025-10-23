#pragma once

#include <filesystem>
#include <string>

namespace fs = std::filesystem;

struct DatabaseInfo {
  std::string port = "";
  std::string server = "";
  std::string file = "";
};

class Config {
 public:
  Config();
  [[nodiscard]] auto get_active_database() const -> DatabaseInfo { return this->active_db; }
  [[nodiscard]] auto has_active_database() const -> bool {
    return !this->active_db.file.empty() && !this->active_db.server.empty();
  }
  void set_active_database(std::string port, std::string server, std::string file) {
    this->active_db.port = port;
    this->active_db.server = server;
    this->active_db.file = file;
  }

  void save() const;

 private:
  fs::path app_folder = fs::path{};
  DatabaseInfo active_db = DatabaseInfo();
};
