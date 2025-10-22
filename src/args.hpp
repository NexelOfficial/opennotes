#pragma once

#include <exception>
#include <map>
#include <string>
#include <vector>

class Args {
 public:
  Args(std::vector<char*> raw_args);
  [[nodiscard]] auto get(const std::string& key) const -> std::string {
    return this->has(key) ? arguments.at(key) : "";
  }
  [[nodiscard]] auto get(int index) const -> std::string {
    return this->get(std::to_string(index));
  }
  [[nodiscard]] auto get_int(const std::string& key) const -> int {
    std::string val = this->get(key);
    try {
      return std::stoi(val);
    } catch (std::exception ex) {
      return 0;
    }
  }
  [[nodiscard]] auto get_int(int index) const -> int {
    return this->get_int(std::to_string(index));
  }

  [[nodiscard]] auto has(const std::string& key) const -> bool {
    auto it = arguments.find(key);
    return it != arguments.end() && !it->second.empty();
  }
  [[nodiscard]] auto has(int index) const -> bool {
    auto it = arguments.find(std::to_string(index));
    return it != arguments.end() && !it->second.empty();
  }

 private:
  std::map<std::string, std::string> arguments = {};
};
