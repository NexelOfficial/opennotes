#pragma once
#include <domino/global.h>
#include <domino/osmisc.h>

#include <exception>
#include <format>
#include <string>

class [[nodiscard]] NotesException : public std::exception {
 public:
  explicit NotesException(STATUS code, const char* prefix) : code(code) {
    this->buffer.resize(MAXWORD);
    OSLoadString(nullptr, code, this->buffer.data(), MAXWORD);

    if (this->buffer.starts_with('\0')) {
      this->buffer = std::to_string(code);
    }

    this->buffer = std::format("{}: {}", prefix, this->buffer);
  }

  [[nodiscard]] auto get_code() const noexcept -> STATUS { return this->code; }
  [[nodiscard]] auto what() const noexcept -> const char* override { return buffer.c_str(); }

 private:
  STATUS code = -1;
  std::string buffer = "";
};