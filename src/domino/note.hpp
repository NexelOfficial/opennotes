#pragma once
#include <domino/dirctx.h>

#include <string>

class Note {
 public:
  Note(DHANDLE db_handle, NOTEID note_id);
  ~Note();
  Note(const Note&) = delete;
  auto operator=(const Note&) = delete;
  Note(Note&&) noexcept = delete; 
  auto operator=(Note&&) noexcept = delete;

  [[nodiscard]] static auto id_to_string(NOTEID id) -> std::string;

  [[nodiscard]] auto is_valid() const -> bool { return this->handle != NULLHANDLE; }
  [[nodiscard]] auto get_handle() const -> DHANDLE { return this->handle; }

 private:
  DHANDLE handle = NULLHANDLE;
  NOTEID note_id = NULL;
};