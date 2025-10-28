#pragma once
#include <domino/global.h>
#include <domino/nsfsearc.h>

#include <array>
#include <map>
#include <string>
#include <vector>

#include "../domino/os.hpp"

class Parser {
 public:
  const static USHORT MAX_ITEM_SIZE = 34816;

  auto static parse_timedate(std::vector<USHORT>& buffer) -> std::string {
    if (buffer.size() != sizeof(TIMEDATE)) return "";

    auto date = *reinterpret_cast<TIMEDATE*>(buffer.data());
    std::array<char, MAXALPHATIMEDATE> raw_text{};
    USHORT raw_text_len = NULL;
    ConvertTIMEDATEToText(nullptr, nullptr, &date, raw_text.data(), MAXALPHATIMEDATE,
                          &raw_text_len);

    return {raw_text.data(), raw_text_len};
  }

  auto static parse_text(std::vector<USHORT>& buffer) -> std::string {
    auto raw_text = reinterpret_cast<const char*>(buffer.data());
    return {raw_text, buffer.size()};
  }

  auto static parse_number(std::vector<USHORT>& buffer) -> double {
    if (buffer.size() != sizeof(double)) return 0;
    return *reinterpret_cast<double*>(buffer.data());
  }

  [[nodiscard]] auto static parse_text_list(std::vector<USHORT>& buffer) -> std::string {
    // Verfy we have data
    if (buffer.size() < 4) {
      return std::string{""};
    }

    // Get the amount of entries
    OSObject obj = OSObject(buffer);
    auto entries = obj.get<USHORT>();
    std::map<USHORT, USHORT> lengths{};

    if (entries > buffer.size()) {
      return std::string{};
    }

    for (USHORT i = 0; i < entries; i++) {
      lengths[i] = obj.get<USHORT>();
    }

    // Concatenate all entries
    std::string output = "";
    for (USHORT i = 0; i < entries; i++) {
      output += obj.get_string(lengths[i]) + ",";
    }

    return output.empty() ? "" : output.substr(0, output.size() - 1);
  }
};
