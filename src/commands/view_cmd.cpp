#include <domino/global.h>
#include <domino/nsferr.h>

#include <format>
#include <future>
#include <iostream>
#include <sstream>

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

auto getDocsInView(const View &collection, int count, int column) -> std::vector<NOTEID> {
  std::vector<NOTEID> note_ids{};
  std::vector<NIFEntry> entries{};

  try {
    entries = collection.get_entries(count ? count : 0xFFFFFFFF);
  } catch (NotesException ex) {
    Log::error(ex.what());
  }

  // Verify we have results
  if (entries.size() == 0) {
    return note_ids;
  }

  for (NIFEntry entry : entries) {
    int final_index = (int)fmin(entry.columns.size() - 1, column);

    if (entry.columns.size() > 0 && column != -1) {
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

      std::cout << "[" << Note::id_to_string(entry.id) << "] " << item_text << "\n";
    }

    note_ids.push_back(entry.id);
  }

  return note_ids;
}

static auto view_thread(DatabaseInfo db_info, std::string formula_str,
                        std::vector<NOTEID> tasks) -> std::vector<std::string> {
  std::vector<std::string> results = {};

  STATUS err = NotesInitExtended(0, nullptr);
  if (err) {
    return results;
  }

  const Database db = Database(db_info.port, db_info.server, db_info.file);
  Formula *formula = nullptr;

  try {
    formula = new Formula(formula_str);
  } catch (const NotesException &ex) {
    Log::error(ex.what());
    return results;
  }

  for (NOTEID task : tasks) {
    try {
      const Note note = Note(db.get_handle(), task);
      std::string output = formula->evaluate(note.get_handle());

      if (output != "") {
        results.push_back("[" + Note::id_to_string(task) + "] " + output);
      }
    } catch (const NotesException &ex) {
      if (ex.get_code() != ERR_INVALID_NOTE) {
        Log::error(ex.what());
      }
    }
  };

  return results;
}

static auto view_cmd(const Args *args, Config *config) -> STATUS {
  std::string view_name = args->get(2);

  if (view_name.empty()) {
    Args::log_usage("view <name>", {"formula", "count", "column"});
    return 0;
  }

  // Get port, server and file from config
  if (!config->has_active_database()) {
    std::cout << "No database is opened.\n";
    Args::log_usage("use <server> <file> [port]", {});
    return (STATUS)NOERROR;
  }

  // Init
  STATUS err = NotesInitExtended(0, nullptr);
  if (err) {
    return Log::error(err, "NotesInitExtended error");
  }

  // Get count and column
  int count = args->get_int("count");
  int column = args->has("column") ? args->get_int("column") : -1;

  // Open database and get view docs
  const DatabaseInfo db_info = config->get_active_database();
  const Database db = Database(db_info.port, db_info.server, db_info.file);
  const View view = db.get_view(view_name);
  const std::vector<NOTEID> note_ids = getDocsInView(view, count, column);

  if (!args->has("formula")) {
    return NOERROR;
  }

  auto ms_start = std::chrono::high_resolution_clock::now();

  // Process note_ids in threads
  int thread_count = (int)fmin(8, fmax(1, note_ids.size() / 20));
  std::vector<std::future<std::vector<std::string>>> futures = {};
  futures.reserve(thread_count);

  for (int thread_index = 0; thread_index < thread_count; thread_index++) {
    size_t total = note_ids.size();
    size_t chunk = (total + thread_count - 1) / thread_count;

    size_t start = thread_index * chunk;
    size_t end = std::min(start + chunk, total);
    std::string formula_str = args->get("formula");
    std::vector<NOTEID> tasks(note_ids.begin() + start, note_ids.begin() + end);

    futures.emplace_back(std::async(std::launch::async, view_thread, db_info, formula_str, tasks));
  }

  size_t printed_items = 0;
  for (auto &f : futures) {
    std::vector<std::string> items = f.get();
    printed_items += items.size();
    for (std::string val : items) {
      std::cout << val << "\n";
    }
  }

  std::cout << "(" << (note_ids.size() - printed_items) << " items didn't return anything)\n";

  auto ms_end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(ms_end - ms_start).count();
  std::cout << std::format("Formula evaluation completed in {} ms.\n", duration);

  return 0;
}

namespace {
const bool registered = [] {
  CommandRegistry::instance().register_command("view", view_cmd);
  return true;
}();
}  // namespace
