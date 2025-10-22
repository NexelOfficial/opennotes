#pragma once
#include <format>

#include <domino/global.h>
#include <domino/osmisc.h>

#include <exception>
#include <string>

class [[nodiscard]] NotesException : public std::exception {
 public:
  explicit NotesException(STATUS code, const char* prefix) : code(code) {
    this->buffer.resize(MAXWORD);
    this->buffer = std::format("{}: ", prefix);
    OSLoadString(nullptr, this->code, this->buffer.data(), MAXWORD);

    if (this->buffer.empty()) {
      this->buffer += std::to_string(this->code);
    }
  }

  [[nodiscard]] auto get_code() const noexcept -> STATUS { return this->code; }
  [[nodiscard]] auto what() const noexcept -> const char* override { return buffer.c_str(); }

 private:
  STATUS code = -1;
  std::string buffer = "";
};