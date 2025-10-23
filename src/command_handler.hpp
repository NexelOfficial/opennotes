#pragma once
#include <domino/global.h>

#include <functional>
#include <string>
#include <unordered_map>

#include "utils/args.hpp"
#include "utils/config.hpp"

using CommandHandler = std::function<STATUS(const Args* args, Config* config)>;

class CommandRegistry {
 public:
  static auto instance() -> CommandRegistry&;

  void register_command(const std::string& name, CommandHandler handler);
  [[nodiscard]] auto get(const std::string& name) const -> CommandHandler;

 private:
  std::unordered_map<std::string, CommandHandler> commands;
};
