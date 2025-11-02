#include <domino/global.h>
#include <domino/nif.h>
#include <domino/nsferr.h>
#include <domino/nsfsearc.h>
#include <domino/xml.h>
#include <minwindef.h>
#include <windows.h>
#include <winerror.h>

#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "command_handler.hpp"
#include "utils/args.hpp"
#include "utils/config.hpp"
#include "utils/log.hpp"


auto main(int argc, char *argv[]) -> int {
  if (argc < 2) {
    std::cerr << "Usage: onotes <command> [args...]\nCommands: view, use";
    return 0;
  }

  auto args = Args(std::vector<char *>(argv, argv + argc));
  auto config = Config();

  // Init
  STATUS err = NotesInitExtended(0, nullptr);
  if (err) {
    return Log::error(err, "NotesInitExtended error");
  }

  std::string command = args.get(1);
  try {
    auto handler = CommandRegistry::instance().get(command);
    return handler(&args, &config);
  } catch (std::exception ex) {
    std::cout << ex.what() << "\n";
  }

  // Finish
  NotesTerm();
}
