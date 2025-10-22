#include "nif_collection.hpp"

#include "../log.hpp"
#include "error.hpp"

NIFCollection::NIFCollection(DHANDLE db_handle, std::string view_name) {
  NOTEID view_note_id = 0;
  STATUS err = NIFFindView(db_handle, view_name.c_str(), &view_note_id);
  if (err != NOERROR) {
    throw NotesException(err, "NIFFindView error");
  }

  err = NIFOpenCollection(db_handle, db_handle, view_note_id, NULL, NULLHANDLE, &this->handle,
                          NULLHANDLE, nullptr, NULLHANDLE, NULLHANDLE);
  if (err != NOERROR) {
    throw NotesException(err, "NIFOpenCollection error");
  }
}

NIFCollection::~NIFCollection() {
  if (this->handle == NULLHANDLE) {
    return;
  }

  STATUS err = NIFCloseCollection(this->handle);
  if (err != NOERROR) {
    Log::error(err, "NIFCloseCollection error");
  }
}

auto NIFCollection::read_entries(COLLECTIONPOSITION* pos, DWORD read_mask,
                                 DWORD return_count) const -> NIFEntries {
  DHANDLE entries_handle = NULLHANDLE;
  DWORD return_length = NULL;

  STATUS err =
      NIFReadEntries(this->handle, pos, NAVIGATE_NEXT, 1, NAVIGATE_NEXT, return_count, read_mask,
                     &entries_handle, nullptr, nullptr, &return_length, nullptr);
  if (err != NOERROR) {
    throw NotesException(err, "NIFReadEntries error");
  }

  return {return_length, entries_handle};
}