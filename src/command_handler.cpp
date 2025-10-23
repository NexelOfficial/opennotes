#include "command_handler.hpp"

#include <stdexcept>

CommandRegistry& CommandRegistry::instance() {
  static CommandRegistry inst;
  return inst;
}

void CommandRegistry::register_command(const std::string& name, CommandHandler handler) {
  this->commands[name] = std::move(handler);
}

auto CommandRegistry::get(const std::string& name) const -> CommandHandler {
  auto it = this->commands.find(name);
  if (it == this->commands.end()) {
    throw std::runtime_error("Unknown command: " + name);
  }
  
  return it->second;
}
