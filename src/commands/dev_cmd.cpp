#include <domino/global.h>
#include <domino/idtable.h>
#include <domino/nsfdb.h>
#include <domino/ostime.h>
#include <ixwebsocket/IXWebSocketServer.h>

#include <chrono>
#include <csignal>
#include <iostream>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <vector>

#include "../command_handler.hpp"
#include "../domino/database.hpp"
#include "../utils/error.hpp"
#include "../utils/log.hpp"
#include "ixwebsocket/IXWebSocket.h"

using WSNoteProps = std::map<std::string, std::string>;

const int WS_PORT = 40456;
const std::string WS_IP = "127.0.0.1";
bool should_exit = false;

void signal_handler(int) { should_exit = true; }

static auto dev_cmd(const Args* args, Config* config) -> STATUS {
  // Get port, server and file from config
  if (!config->has_active_database()) {
    std::cout << "No database is opened.\n";
    Args::log_usage("use <server> <file> [port]", {});
    return (STATUS)NOERROR;
  }

  int actual_port = args->has("port") ? args->get_int("port") : WS_PORT;
  std::vector<std::weak_ptr<ix::WebSocket>> clients = {};

  ix::initNetSystem();
  ix::WebSocketServer server(actual_port, WS_IP);
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
  signal(SIGINT, signal_handler);

  try {
    const DatabaseInfo db_info = config->get_active_database();
    const Database db = Database(db_info.server, db_info.file, db_info.port);

    Log::update("development server started");
    Log::info("running on ", Log::blue, WS_IP, ":", actual_port);

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
        std::vector<WSNoteProps> outputs = {};

        for (const auto& note_id : new_changes) {
          // Construct output
          WSNoteProps props = {};
          props["noteid"] = Note::id_to_string(note_id);

          try {
            const Note note = db.get_note(note_id);

            std::array<char, 256> field = {};
            NSFItemConvertToText(note.get_handle(), "$TITLE", field.data(), field.size(), ';');
            props["title"] = std::string(field.data(), strlen(field.data()));
          } catch (const NotesException& ex) {
            Log::error(ex.what());
          }

          outputs.emplace_back(props);
        }

        // Log output to console
        std::string all_changes;
        for (size_t i = 0; i < outputs.size(); ++i) {
          size_t left = new_changes.size() - i;
          if (i == 2 && left > 0) {
            all_changes += std::to_string(left) + " more...";
            break;
          }

          all_changes += outputs[i]["noteid"];

          std::string title = outputs[i]["title"];
          if (!title.empty()) {
            all_changes += std::format(" ({})", title.substr(0, title.find(";")));
          }

          if (i != new_changes.size() - 1) {
            all_changes += ", ";
          }
        }
        Log::update(all_changes);

        nlohmann::json j;
        j["event"] = "DB_UPDATE";
        j["changes"] = outputs;

        // Send output to client
        for (auto it = clients.begin(); it != clients.end();) {
          if (it->expired()) {
            it = clients.erase(it);
          } else {
            it->lock()->send(j.dump(2));
            ++it;
          }
        }
      }

      if (should_exit) {
        break;
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
