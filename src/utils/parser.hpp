#pragma once
#include <domino/global.h>
#include <domino/nsfsearc.h>

#include <array>
#include <string>
#include <vector>

class Parser {
 public:
  auto static parse_timedate(std::vector<USHORT>& buffer) -> std::string {
    if (buffer.size() != sizeof(TIMEDATE)) return "";

    auto date = *reinterpret_cast<TIMEDATE*>(buffer.data());
    std::array<char, MAXALPHATIMEDATE> raw_text{};
    WORD raw_text_len = NULL;
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
};
