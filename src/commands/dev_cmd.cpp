#include <domino/global.h>
#include <domino/nsfdb.h>
#include <domino/nsferr.h>
#include <domino/ostime.h>
#include <ixwebsocket/IXWebSocketServer.h>
#include <wincrypt.h>
#include <winsock.h>

#include <chrono>
#include <iostream>
#include <termcolor/termcolor.hpp>
#include <thread>

#include "../command_handler.hpp"
#include "../domino/database.hpp"
#include "../utils/error.hpp"
#include "../utils/log.hpp"
#include "ixwebsocket/IXWebSocket.h"

static auto dev_cmd(const Args* args, Config* config) -> STATUS {
  // Get port, server and file from config
  if (!config->has_active_database()) {
    std::cout << "No database is opened.\n";
    Args::log_usage("use <server> <file> [port]", {});
    return (STATUS)NOERROR;
  }

  std::vector<std::weak_ptr<ix::WebSocket>> clients = {};

  ix::initNetSystem();
  ix::WebSocketServer server(40456, "127.0.0.1");
  server.setOnConnectionCallback(
      [&server, &clients](std::weak_ptr<ix::WebSocket> webSocket,
                          std::shared_ptr<ix::ConnectionState> connectionState) -> void {
        clients.push_back(webSocket);

        auto ws = webSocket.lock();
        if (ws) {
          ws->setOnMessageCallback(
              [webSocket, connectionState,
               &server](const std::unique_ptr<ix::WebSocketMessage>& msg) -> void { return; });
        }
      });

  auto res = server.listenAndStart();

  try {
    const DatabaseInfo db_info = config->get_active_database();
    const Database db = Database(db_info.server, db_info.file, db_info.port);

    std::cout << termcolor::bright_green << "Development server running" << termcolor::reset
              << ":\n> IP: " << termcolor::bright_blue << "http://127.0.0.1:40456/\n\n";

    TIMEDATE previous_modified = {};
    while (true) {
      TIMEDATE now = {};
      OSCurrentTIMEDATE(&now);

      TIMEDATE last_modified = {};
      NSFDbModifiedTime(db.get_handle(), nullptr, &last_modified);
      bool is_same = (last_modified.Innards[0] == previous_modified.Innards[0]) &&
                     (last_modified.Innards[1] == previous_modified.Innards[1]);

      if (!is_same) {
        previous_modified = last_modified;

        for (auto it = clients.begin(); it != clients.end();) {
          if (it->expired()) {
            it = clients.erase(it);
          } else {
            it->lock()->send("DB_UPDATE");
            ++it;
          }
        }

        std::cout << termcolor::grey << "[change]" << termcolor::bright_blue << " Emitted to "
                  << clients.size() << " client(s)\n";
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
  } catch (const NotesException& ex) {
    Log::error(ex.what());
  }

  ix::uninitNetSystem();
  return 0;
}

namespace {
const bool registered = [] {
  CommandRegistry::instance().register_command("dev", dev_cmd);
  return true;
}();
}  // namespace
