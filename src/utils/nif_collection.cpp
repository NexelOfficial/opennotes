#include "nif_collection.hpp"

#include <minwindef.h>

#include <vector>

#include "../log.hpp"
#include "error.hpp"
#include "os.hpp"

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

auto NIFCollection::read_entries(COLLECTIONPOSITION *pos, DWORD return_count,
                                 DWORD read_mask) const -> std::vector<NIFEntry> {
  DHANDLE entries_handle = NULLHANDLE;
  DWORD entries_length = NULL;

  STATUS err =
      NIFReadEntries(this->handle, pos, NAVIGATE_NEXT, 1, NAVIGATE_NEXT, return_count, read_mask,
                     &entries_handle, nullptr, nullptr, &entries_length, nullptr);

  if (err != NOERROR) {
    throw NotesException(err, "NIFReadEntries error");
  }

  // Verify we have results
  std::vector<NIFEntry> entries{};
  if (entries_length == 0) {
    return entries;
  }

  // Lock the object and read it
  auto entries_obj = new OSObject(entries_handle);
  for (DWORD i = 0; i < entries_length; ++i) {
    // Read noteid
    auto note_id = entries_obj->get<NOTEID>();
    entries_obj->inc(sizeof(NOTEID));
    entries.push_back({note_id, {}});

    // Read item table
    auto summary = entries_obj->get_raw<BYTE *>() + sizeof(ITEM_VALUE_TABLE);
    auto item_table = entries_obj->get<ITEM_VALUE_TABLE>();
    entries_obj->inc(item_table.Length);

    // Get the length of each item in the table
    std::vector<USHORT> item_lengths{};
    for (USHORT j = 0; j < item_table.Items; j++) {
      item_lengths.push_back(*(USHORT *)summary);
      summary += sizeof(USHORT);
    }

    // Get the items in the table
    for (USHORT j = 0; j < item_table.Items; j++) {
      // Skip empty items
      USHORT length = item_lengths[j];
      if (length == 0) {
        continue;
      }

      // Get the item type
      USHORT type = *(USHORT *)summary;
      summary += sizeof(USHORT);

      // Get the item data
      std::vector<USHORT> item_buffer{};
      item_buffer.resize(length - sizeof(USHORT));
      memcpy(item_buffer.data(), summary, length - sizeof(USHORT));

      // Advance to the next item
      summary += length - sizeof(USHORT);
      entries[i].columns.push_back({type, item_buffer});
    }
  }

  entries_obj->unlock_and_free();
  return entries;
}