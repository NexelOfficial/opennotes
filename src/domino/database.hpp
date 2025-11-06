#pragma once
#include <domino/global.h>

#include <optional>
#include <string>

#include "note.hpp"
#include "view.hpp"

class Database {
 public:
  Database(std::string server, std::string file) : Database(std::string{}, server, file) {}
  Database(std::string server, std::string file, std::optional<std::string> port);
  Database(Database& other) = delete;
  ~Database();
  Database(const Database&) = delete;
  auto operator=(const Database&) = delete;
  Database(Database&&) noexcept = delete;
  auto operator=(Database&&) noexcept = delete;

  [[nodiscard]] auto get_view(const std::string& name) const -> View {
    return {this->handle, name};
  }

  [[nodiscard]] auto get_note(NOTEID note_id) const -> Note { return {this->handle, note_id}; }

  [[nodiscard]] auto is_valid() const -> bool { return this->handle != NULLHANDLE; }
  [[nodiscard]] auto get_handle() const -> DHANDLE { return this->handle; }

 private:
  DHANDLE handle = NULLHANDLE;
};