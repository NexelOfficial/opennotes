#include <domino/global.h>
#include <domino/idtable.h>
#include <domino/nsfdb.h>
#include <domino/nsferr.h>
#include <domino/ostime.h>
#include <ixwebsocket/IXWebSocketServer.h>
#include <wincrypt.h>
#include <winsock.h>

#include <chrono>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <termcolor/termcolor.hpp>
#include <thread>
#include <vector>

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

    std::map<NOTEID, int> old_changes = {};

    int tick = 0;
    while (++tick) {
      TIMEDATE since = {};
      TIMEDATE until = {};
      OSCurrentTIMEDATE(&since);
      TimeDateAdjust(&since, -2, 0, 0, 0, 0, 0);

      DHANDLE output_table = NULLHANDLE;
      NSFDbGetModifiedNoteTable(db.get_handle(), NOTE_CLASS_ALL, since, &until, &output_table);

      // Add new changes
      const DWORD num_entries = IDEntries(output_table);
      std::vector<NOTEID> new_changes = {};
      if (num_entries) {
        DWORD num_scanned = 0;
        NOTEID note_id = NULLHANDLE;

        while (IDScan(output_table, num_scanned++ == 0, &note_id)) {
          if (!old_changes.contains(note_id)) {
            old_changes[note_id] = tick;
            new_changes.push_back(note_id);
          }
        }
      }

      IDDestroyTable(output_table);

      // Remove the old ones
      for (auto& pair : old_changes) {
        if (tick - pair.second > 10) {
          old_changes.erase(pair.first);
        }
      }

      // Check for new changes
      if (new_changes.size() > 0) {
        for (auto it = clients.begin(); it != clients.end();) {
          if (it->expired()) {
            it = clients.erase(it);
          } else {
            nlohmann::json j;

            std::vector<std::string> forms;
            for (const auto& note_id : new_changes) {
              try {
                const Note note = db.get_note(note_id);

                std::array<char, 256> field = {};
                NSFItemConvertToText(note.get_handle(), "$TITLE", field.data(), field.size(), ';');
                std::string outp = std::string(field.data(), strlen(field.data()));
                forms.emplace_back(outp);
              } catch (const NotesException&) {
                forms.emplace_back("");
              }
            }

            j["event"] = "DB_UPDATE";
            j["changes"] = forms;
            it->lock()->send(j.dump(2));
            ++it;
          }
        }

        std::string all_changes;
        for (size_t i = 0; i < new_changes.size(); ++i) {
          size_t left = new_changes.size() - i;
          if (i == 3 && left > 0) {
            all_changes += std::to_string(left) + " more...";
            break;
          }

          all_changes += Note::id_to_string(new_changes[i]);
          if (i != new_changes.size() - 1) {
            all_changes += ", ";
          }
        }

        std::cout << termcolor::grey << "[onotes]" << termcolor::bright_green << " update "
                  << termcolor::reset << all_changes << " \n"
                  << termcolor::reset;
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
