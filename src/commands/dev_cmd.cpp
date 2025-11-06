#include <domino/global.h>
#include <domino/nsferr.h>

#include <format>
#include <future>
#include <iostream>

#include "../command_handler.hpp"
#include "../domino/database.hpp"
#include "../domino/view.hpp"
#include "../utils/error.hpp"
#include "../utils/log.hpp"
#include "../utils/parser.hpp"

static auto dev_cmd(const Args* args, Config* config) -> STATUS {
  // Get port, server and file from config
  if (!config->has_active_database()) {
    std::cout << "No database is opened.\n";
    Args::log_usage("use <server> <file> [port]", {});
    return (STATUS)NOERROR;
  }

  return 0;
}

namespace {
const bool registered = [] {
  CommandRegistry::instance().register_command("dev", dev_cmd);
  return true;
}();
}  // namespace
