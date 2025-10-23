#include "config.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

Config::Config() {
  const char* user_profile = std::getenv("USERPROFILE");
  if (!user_profile) {
    throw std::runtime_error("Could not get user profile path.");
  }

  this->app_folder = fs::path(user_profile) / ".opennotes";

  // Create folder if it doesn't exist
  if (!fs::exists(app_folder)) {
    if (!fs::create_directory(app_folder)) {
      throw std::runtime_error("Could not create config directory.");
    }
  }

  // Read config
  fs::path configFile = app_folder / "config.json";
  if (!fs::exists(configFile)) {
    return;
  }

  std::ifstream file(configFile);
  if (!file.is_open()) {
    throw std::runtime_error("Unable to open config file.");
  }

  nlohmann::json j;
  file >> j;

  if (j.contains("active_db")) {
    this->active_db.port = j["active_db"]["port"].get<std::string>();
    this->active_db.server = j["active_db"]["server"].get<std::string>();
    this->active_db.file = j["active_db"]["file"].get<std::string>();
  }

  // Close the file
  file.close();
}

void Config::save() const {
  nlohmann::json j;

  j["active_db"]["port"] = this->active_db.port;
  j["active_db"]["server"] = this->active_db.server;
  j["active_db"]["file"] = this->active_db.file;

  fs::path config_file = app_folder / "config.json";
  std::ofstream file(config_file);
  if (!file.is_open()) {
    std::runtime_error("Unable to open config file for writing.");
  }

  file << j.dump(2);
  file.close();
}