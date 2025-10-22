#include <domino/global.h>
#include <domino/nif.h>
#include <domino/nsferr.h>
#include <domino/nsfsearc.h>
#include <domino/xml.h>
#include <minwindef.h>
#include <windows.h>
#include <winerror.h>

#include <cstdlib>
#include <format>
#include <iostream>
#include <string>
#include <vector>

#include "args.hpp"
#include "log.hpp"
#include "utils/database.hpp"
#include "utils/error.hpp"
#include "utils/formula.hpp"
#include "utils/nif_collection.hpp"
#include "utils/note.hpp"
#include "utils/parser.hpp"

Args *args = nullptr;

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
    int count = args->get_int("count");
    entries = collection.read_entries(&coll_pos, count ? count : 0xFFFFFFFF);
  } catch (NotesException ex) {
    Log::error(ex.what());
  }

  // Verify we have results
  if (entries.size() == 0) {
    return note_ids;
  }

  for (NIFEntry entry : entries) {
    int column_index = args->get_int("column");
    int final_index = (int)fmin(entry.columns.size() - 1, column_index);
    NIFColumn col = entry.columns[final_index];

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

    std::cout << entry.id << " | " << item_text << "\n";
    note_ids.push_back(entry.id);
  }

  return note_ids;
}

auto main(int argc, char *argv[]) -> int {
  args = new Args(std::vector<char *>(argv, argv + argc));

  // Init
  STATUS err = NotesInitExtended(argc, argv);
  if (err) {
    return Log::error(err, "NotesInitExtended error");
  }

  // Get port, server and file from env
  const char *db_port = std::getenv("DOMINO_PORT");
  const char *db_server = std::getenv("DOMINO_SERVER");
  const char *db_file = std::getenv("DOMINO_FILE");

  if (db_port == nullptr) {
    db_port = "";
  }

  if (db_server == nullptr || db_file == nullptr) {
    Log::error("Required environment variables DOMINO_SERVER and DOMINO_FILE aren't set.\n");
    return 0;
  }

  // Open database
  Database db = Database(db_port, db_server, db_file);

  // Get design elements
  std::string view_name = args->get(1);
  std::vector<NOTEID> note_ids = getDocsInView(db.get_handle(), argv[1]);

  // Check for formula
  if (args->has("formula")) {
    Formula *formula = nullptr;

    try {
      formula = new Formula(args->get("formula"));
    } catch (const NotesException &ex) {
      Log::error(ex.what());
      return 0;
    }

    for (auto note_id : note_ids) {
      try {
        Note note = Note(db.get_handle(), note_id);
        std::string output = formula->evaluate(note.get_handle());
        std::cout << "- " << output << "\n";
      } catch (const NotesException &ex) {
        if (ex.get_code() != ERR_INVALID_NOTE) Log::error(ex.what());
      }
    }
  }

  return 0;
}
