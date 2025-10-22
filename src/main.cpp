#include <domino/global.h>
#include <domino/nif.h>
#include <domino/nsferr.h>
#include <domino/nsfsearc.h>
#include <domino/xml.h>
#include <minwindef.h>
#include <windows.h>
#include <winerror.h>

#include <iostream>
#include <string>
#include <vector>
#include <array>

#include "log.hpp"
#include "utils/database.hpp"
#include "utils/error.hpp"
#include "utils/formula.hpp"
#include "utils/nif_collection.hpp"
#include "utils/note.hpp"
#include "utils/os.hpp"

STATUS LNCALLBACK xmlWriter(const char *pBuffer, DWORD_PTR dwBytesWritten, void *pAction) {
  auto *buffer = static_cast<std::string *>(pAction);
  buffer->append(pBuffer, dwBytesWritten);
  return NOERROR;
}

STATUS LNCALLBACK collectDesignNotes(void *ctxVoid, SEARCH_MATCH *match, ITEM_TABLE *table) {
  auto *ctx = reinterpret_cast<std::vector<NOTEID> *>(ctxVoid);
  ctx->push_back(match->ID.NoteID);
  return NOERROR;
};

auto getDocsInView(DHANDLE db_handle, std::string view_name) -> std::vector<NOTEID> {
  std::vector<NOTEID> note_ids{};
  std::vector<NIFEntry> entries{};

  try {
    std::cout << view_name << "\n";
    NIFCollection collection = NIFCollection(db_handle, view_name);

    // Start at the beginning of the view
    COLLECTIONPOSITION coll_pos;
    coll_pos.Level = 0;
    coll_pos.Tumbler[0] = 0;

    // Read the entries at the current postion
    entries = collection.read_entries(&coll_pos);
  } catch (NotesException ex) {
    Log::error(ex.what());
  }

  // Verify we have results
  if (entries.size() == 0) {
    return note_ids;
  }

  for (NIFEntry entry : entries) {
    NIFData data = entry.data[fmin(1, entry.data.size() - 1)];

    // Read the item
    if (data.type == TYPE_TEXT && data.buffer.size() > 0) {
      auto raw_text = reinterpret_cast<const char *>(data.buffer.data());
      std::string item_text(raw_text, data.buffer.size());
      std::cout << item_text << "\n";
    } else if (data.type == TYPE_TIME) {
      std::array<char, 100> raw_text{};
      WORD raw_text_len = NULL;
      TIMEDATE date = *reinterpret_cast<TIMEDATE *>(data.buffer.data());
      ConvertTIMEDATEToText(nullptr, nullptr, &date, raw_text.data(), MAXALPHATIMEDATE,
                            &raw_text_len);

      std::string item_text(raw_text.data(), raw_text_len);
      // std::cout << item_text << "\n";
    } else if (data.type == TYPE_NUMBER) {
      NUMBER raw_number = *(NUMBER*)data.buffer.data();
      std::cout << raw_number << "\n";
    } else {
      std::cout << data.buffer.size() << " with type " << data.type << "\n";
    }

    note_ids.push_back(entry.id);
  }

  return note_ids;
}

auto main(int argc, char *argv[]) -> int {
  // Init
  STATUS err = NotesInitExtended(argc, argv);
  if (err) {
    return Log::error(err, "NotesInitExtended error");
  }

  // Open database
  Database db = Database("Aspew-6/Asperience", "nathanscoolefreestyle/fs3_site.nsf");

  // Get design elements
  std::vector<NOTEID> note_ids = getDocsInView(db.get_handle(), argv[1]);

  // Create exporter
  DXLEXPORTHANDLE dxl_handle = NULLHANDLE;
  err = DXLCreateExporter(&dxl_handle);
  if (err) {
    return Log::error(err, "DXLCreateExporter error");
  }

  // Export the notes
  // Formula formula = Formula(std::string(argv[2]));

  // for (auto note_id : note_ids) {
  //   try {
  //     Note note = Note(db.get_handle(), note_id);
  //     std::string output = formula.evaluate(note.get_handle());
  //     std::cout << "- " << output << "\n";
  //   } catch (const NotesException &ex) {
  //     if (ex.get_code() != ERR_INVALID_NOTE) Log::error(ex.what());
  //   }
  // }

  DXLDeleteExporter(dxl_handle);
  return 0;
}
