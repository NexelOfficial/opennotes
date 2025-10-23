#include <iostream>

#include "../command_handler.hpp"

static auto use_cmd(const Args* args, Config* config) -> STATUS {
  std::string db_server = args->get(2);
  std::string db_file = args->get(3);
  std::string db_port = args->get(4);

  if (db_server.empty() || db_file.empty()) {
    Args::log_usage("use <server> <file> [port]", {});
    return 0;
  }

  config->set_active_database(db_port, db_server, db_file);
  config->save();
  std::cout << "Selected database: '" << db_file << "'\n";
  return 0;
}

namespace {
const bool registered = [] {
  CommandRegistry::instance().register_command("use", use_cmd);
  return true;
}();
}  // namespace
