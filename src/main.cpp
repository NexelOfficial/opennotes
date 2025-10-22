#include <windows.h>
#include <winerror.h>

#include <iostream>
#include <string>
#include <vector>

#include <domino/global.h>
#include <domino/nif.h>
#include <domino/nsferr.h>
#include <domino/nsfsearc.h>
#include <domino/xml.h>

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

auto getDocsInView(DHANDLE db_handle, std::string view_name,
                   std::vector<NOTEID> &note_ids) -> STATUS {
  NIFCollection collection = NIFCollection(db_handle, view_name);

  // Start at the beginning of the view
  COLLECTIONPOSITION coll_pos;
  coll_pos.Level = 0;
  coll_pos.Tumbler[0] = 0;

  // Loop through the entries
  while (true) {
    DWORD found = NULL;
    DHANDLE entries_buf = NULLHANDLE;

    STATUS err = NIFReadEntries(collection.get_handle(), &coll_pos, NAVIGATE_NEXT, 1, NAVIGATE_NEXT,
                                100, READ_MASK_NOTEID | READ_MASK_SUMMARYVALUES, &entries_buf,
                                nullptr, nullptr, &found, nullptr);

    if (err != NOERROR || found == 0) break;

    auto entries_obj = new OSObject(entries_buf);
    for (DWORD i = 0; i < found; ++i) {
      // Read noteid
      auto note_id = entries_obj->get<NOTEID>();
      entries_obj->inc(sizeof(NOTEID));

      // Skip item table
      auto item_table = entries_obj->get<ITEM_VALUE_TABLE>();
      entries_obj->inc(item_table.Length);

      note_ids.push_back(note_id);
    }

    entries_obj->unlock_and_free();

    // Move the collection position forward
    coll_pos.Tumbler[0] += found;
  }

  return NOERROR;
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
  std::vector<NOTEID> note_ids{};
  err = getDocsInView(db.get_handle(), argv[1], note_ids);
  if (err) {
    return err;
  }

  // Create exporter
  DXLEXPORTHANDLE dxl_handle = NULLHANDLE;
  err = DXLCreateExporter(&dxl_handle);
  if (err) {
    return Log::error(err, "DXLCreateExporter error");
  }

  // Export the notes
  std::cout << "HEHH\n";
  Formula formula = Formula(std::string(argv[2]));

  for (auto note_id : note_ids) {
    try {
      Note note = Note(db.get_handle(), note_id);
      std::string output = formula.evaluate(note.get_handle());
      std::cout << "- " << output << "\n";
    } catch (const NotesException &ex) {
      if (ex.get_code() != ERR_INVALID_NOTE) Log::error(ex.what());
    }

    // std::string outputXml = "";
    // err = DXLExportNote(dxl_handle, reinterpret_cast<XML_WRITE_FUNCTION>(xmlWriter), note_handle,
    //                     (void *)&outputXml);
    // if (err) {
    //   return logError(err, "DXLExportNote error");
    // }

    // // Parse to XML
    // Xml::saveDominoNote(outputXml);
  }

  DXLDeleteExporter(dxl_handle);
  return 0;
}
