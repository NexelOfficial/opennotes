#include <domino/global.h>
#include <domino/nif.h>
#include <domino/nsferr.h>
#include <domino/nsfsearc.h>
#include <domino/xml.h>
#include <minwindef.h>
#include <windows.h>
#include <winerror.h>

#include <format>
#include <iostream>
#include <string>
#include <vector>

#include "log.hpp"
#include "utils/database.hpp"
#include "utils/error.hpp"
#include "utils/formula.hpp"
#include "utils/nif_collection.hpp"
#include "utils/note.hpp"
#include "utils/os.hpp"
#include "utils/parser.hpp"

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
    NIFColumn col = entry.columns[0];

    // Read the item
    std::string item_text = "";
    if (col.type == TYPE_TEXT && col.buffer.size() > 0) {
      item_text = Parser::parse_text(col.buffer);
    } else if (col.type == TYPE_TIME) {
      item_text = Parser::parse_timedate(col.buffer);
    } else if (col.type == TYPE_NUMBER) {
      double item_num = Parser::parse_number(col.buffer);
      item_text = std::format("Number: {}", item_num);
    }

    std::cout << item_text << "\n";
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
