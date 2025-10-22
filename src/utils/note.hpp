#pragma once
#include <domino/dirctx.h>

class Note {
 public:
  Note(DHANDLE db_handle, NOTEID note_id);
  ~Note();

  [[nodiscard]] auto is_valid() const -> bool { return this->handle != NULLHANDLE; }
  [[nodiscard]] auto get_handle() const -> DHANDLE { return this->handle; }

 private:
  DHANDLE handle = NULLHANDLE;
  NOTEID note_id = NULL;
};