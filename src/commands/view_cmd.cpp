#include <domino/global.h>
#include <domino/nsferr.h>

#include <format>
#include <iostream>

#include "../command_handler.hpp"
#include "../domino/database.hpp"
#include "../domino/formula.hpp"
#include "../domino/note.hpp"
#include "../domino/view.hpp"
#include "../utils/error.hpp"
#include "../utils/log.hpp"
#include "../utils/parser.hpp"

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

auto getDocsInView(DHANDLE db_handle, std::string view_name, int count,
                   int column) -> std::vector<NOTEID> {
  std::vector<NOTEID> note_ids{};
  std::vector<NIFEntry> entries{};

  try {
    View collection = View(db_handle, view_name);

    // Start at the beginning of the view
    COLLECTIONPOSITION coll_pos;
    coll_pos.Level = 0;
    coll_pos.Tumbler[0] = 0;

    // Read the entries at the current postion
    entries = collection.read_entries(&coll_pos, count ? count : 0xFFFFFFFF);
  } catch (NotesException ex) {
    Log::error(ex.what());
  }

  // Verify we have results
  if (entries.size() == 0) {
    return note_ids;
  }

  for (NIFEntry entry : entries) {
    int final_index = (int)fmin(entry.columns.size() - 1, column);
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
    } else if (col.type == TYPE_TEXT_LIST) {
      item_text = Parser::parse_text_list(col.buffer);
    } else {
      item_text = std::format("Type: {}", col.type);
    }

    std::cout << entry.id << " | " << item_text << "\n";
    note_ids.push_back(entry.id);
  }

  return note_ids;
}

static auto view_cmd(const Args *args, Config *config) -> STATUS {
  std::string view_name = args->get(2);

  if (view_name.empty()) {
    Args::log_usage("view <name>", {"formula", "count", "column"});
    return 0;
  }

  // Init
  STATUS err = NotesInitExtended(0, nullptr);
  if (err) {
    return Log::error(err, "NotesInitExtended error");
  }

  // Get port, server and file from config
  if (!config->has_active_database()) {
    std::cout << "No database is opened.\n";
    Args::log_usage("use <server> <file> [port]", {});
    return 0;
  }

  // Get count and column
  int count = args->get_int("count");
  int column = args->get_int("column");

  // Open database and get view docs
  const DatabaseInfo db_info = config->get_active_database();
  const Database db = Database(db_info.port, db_info.server, db_info.file);
  const std::vector<NOTEID> note_ids = getDocsInView(db.get_handle(), view_name, count, column);

  // Check for formula
  if (args->has("formula")) {
    Formula *formula = nullptr;

    try {
      formula = new Formula(args->get("formula"));
    } catch (const NotesException &ex) {
      Log::error(ex.what());
      return 0;
    }

    if (note_ids.size() > 20) {
      std::cout << std::format(
          "Warning: You're evaluating Formula over a large amount of Note's ({}). Processing can "
          "take significantly longer.",
          note_ids.size());
    }

    for (auto note_id : note_ids) {
      try {
        const Note note = Note(db.get_handle(), note_id);
        std::string output = formula->evaluate(note.get_handle());
        std::cout << "- " << output << "\n";
      } catch (const NotesException &ex) {
        if (ex.get_code() != ERR_INVALID_NOTE) Log::error(ex.what());
      }
    }
  }

  return 0;
}

namespace {
const bool registered = [] {
  CommandRegistry::instance().register_command("view", view_cmd);
  return true;
}();
}  // namespace
