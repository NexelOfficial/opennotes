#include "args.hpp"

#include <regex>

Args::Args(std::vector<char*> raw_args) {
  int keyLessCount = 0;
  std::string key = "";

  for (auto& raw_arg : raw_args) {
    std::string arg = raw_arg;

    if (std::regex_match(arg, std::regex(R"(--\w+)"))) {
      key = arg.substr(2);
    } else {
      if (!key.empty()) {
        this->arguments[key] = arg;
        key = "";
      } else {
        this->arguments[std::to_string(keyLessCount)] = arg;
        keyLessCount++;
      }
    }
  }
}