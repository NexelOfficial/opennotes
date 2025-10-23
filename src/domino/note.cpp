#include "note.hpp"

#include <string>
#include <domino/nsfnote.h>

#include "../utils/error.hpp"
#include "../utils/log.hpp"

Note::Note(DHANDLE db_handle, NOTEID note_id) {
  STATUS err = NSFNoteOpenExt(db_handle, note_id, OPEN_NOOBJECTS, &this->handle);
  if (err != NOERROR) {
    throw NotesException(err, "NSFNoteOpenExt error");
  }
}

Note::~Note() {
  if (this->handle == NULLHANDLE) {
    return;
  }

  STATUS err = NSFNoteClose(this->handle);
  if (err != NOERROR) {
    Log::error(err, "NSFNoteClose error");
  }
}