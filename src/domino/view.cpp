#include "view.hpp"

#include <minwindef.h>

#include <vector>

#include "../utils/error.hpp"
#include "../utils/log.hpp"
#include "os.hpp"

View::View(DHANDLE db_handle, std::string view_name) {
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

View::~View() {
  if (this->handle == NULLHANDLE) {
    return;
  }

  STATUS err = NIFCloseCollection(this->handle);
  if (err != NOERROR) {
    Log::error(err, "NIFCloseCollection error");
  }
}

auto View::get_entries(DWORD return_count) const -> std::vector<NIFEntry> {
  std::vector<NIFEntry> entries{};

  // Start at the beginning of the view
  COLLECTIONPOSITION coll_pos;
  coll_pos.Level = 0;
  coll_pos.Tumbler[0] = 0;

  WORD signal = SIGNAL_MORE_TO_DO;
  while (signal & SIGNAL_MORE_TO_DO) {
    DHANDLE entries_handle = NULLHANDLE;
    DWORD entries_length = NULL;
    STATUS err = NIFReadEntries(this->handle, &coll_pos, NAVIGATE_NEXT, 1, NAVIGATE_NEXT,
                                return_count, READ_MASK_NOTEID | READ_MASK_SUMMARYVALUES,
                                &entries_handle, nullptr, nullptr, &entries_length, &signal);

    if (err != NOERROR) {
      throw NotesException(err, "NIFReadEntries error");
    }

    // Verify we have results
    if (entries_length == 0 || entries_handle == NULLHANDLE) {
      return entries;
    }

    // Lock the object and read it
    auto entries_obj = new OSObject(entries_handle);
    for (DWORD i = 0; i < entries_length; ++i) {
      // Read noteid
      auto note_id = entries_obj->get<NOTEID>();
      entries.push_back({note_id, {}});

      // Read item table
      auto item_base = entries_obj->get_raw<BYTE *>();
      auto item_table = entries_obj->get<ITEM_VALUE_TABLE>();

      // Get the length of each item in the table
      std::vector<USHORT> item_lengths{};
      for (USHORT j = 0; j < item_table.Items; j++) {
        item_lengths.push_back(entries_obj->get<USHORT>());
      }

      // Get the items in the table
      for (USHORT j = 0; j < item_table.Items; j++) {
        // Skip empty items
        USHORT length = item_lengths[j];
        if (length == 0) {
          continue;
        }

        // Get the item type
        auto type = entries_obj->get<USHORT>();

        // Get the item data
        USHORT vec_len = length - sizeof(USHORT);
        auto item_buffer = entries_obj->get<USHORT>(vec_len);

        entries[i].columns.push_back({type, item_buffer});
      }

      entries_obj->mov(item_base + item_table.Length);
    }

    return_count -= entries_length;
  }

  return entries;
}