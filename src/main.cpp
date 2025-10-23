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

#include "utils/args.hpp"
#include "utils/config.hpp"
#include "command_handler.hpp"

auto main(int argc, char *argv[]) -> int {
  if (argc < 2) {
    std::cerr << "Usage: onotes <command> [args...]\nCommands: view, use";
    return 0;
  }

  auto args = new Args(std::vector<char *>(argv, argv + argc));
  auto config = new Config();

  // Set the open database
  std::string command = args->get(1);
  auto handler = CommandRegistry::instance().get(command);
  return handler(args, config);
}
