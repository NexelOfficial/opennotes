#include "database.hpp"

#include <domino/nsfdb.h>
#include <domino/osfile.h>

#include <array>
#include <string>

#include "../utils/error.hpp"
#include "../utils/log.hpp"

Database::Database(std::string server, std::string file, std::optional<std::string> port) {
  std::array<char, MAXWORD> db_path{};
  STATUS err = OSPathNetConstruct(port.has_value() ? port->data() : "", server.data(), file.data(),
                                  db_path.data());

  if (err) {
    throw NotesException(err, "OSPathNetConstruct error");
  }

  err = NSFDbOpen(db_path.data(), &this->handle);
  if (err) {
    throw NotesException(err, "NSFDbOpen error");
  }
}

Database::~Database() {
  if (this->handle == NULLHANDLE) {
    return;
  }

  STATUS err = NSFDbClose(this->handle);
  if (err != NOERROR) {
    Log::error(err, "NSFDbClose error");
  }
}