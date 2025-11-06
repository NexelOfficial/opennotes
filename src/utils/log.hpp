#pragma once
#include <domino/dirctx.h>
#include <domino/osmisc.h>

#include <iomanip>
#include <ostream>
#include <string>
#include <termcolor/termcolor.hpp>

using Manipulator = std::ostream& (*)(std::ostream&);

class Log {
 public:
  template <typename... Args>
  static void info(Args&&... args) {
    std::cout << onotes_prefix << info_label;
    ((std::cout << args), ...);
    std::cout << suffix;
  }

  template <typename... Args>
  static void update(Args&&... args) {
    std::cout << onotes_prefix << update_label;
    ((std::cout << args), ...);
    std::cout << suffix;
  }

  template <typename... Args>
  static void error(Args&&... args) {
    std::cout << onotes_prefix << error_label;
    ((std::cout << args), ...);
    std::cout << suffix;
  }

  static auto error(STATUS code, std::string prefix) -> STATUS;

  inline static Manipulator blue = static_cast<Manipulator>(termcolor::blue);
  inline static Manipulator red = static_cast<Manipulator>(termcolor::red);
  inline static Manipulator green = static_cast<Manipulator>(termcolor::green);

 private:
  template <typename CharT>
  static auto onotes_prefix(std::basic_ostream<CharT>& stream) -> std::basic_ostream<CharT>& {
    return stream << termcolor::grey << "[onotes] " << termcolor::reset;
  }

  template <typename CharT>
  static auto label_padding(std::basic_ostream<CharT>& stream) -> std::basic_ostream<CharT>& {
    return stream << std::left << std::setw(8) << std::setfill(' ');
  }

  template <typename CharT>
  static auto suffix(std::basic_ostream<CharT>& stream) -> std::basic_ostream<CharT>& {
    return stream << termcolor::reset << " \n";
  }

  template <typename CharT>
  static auto error_label(std::basic_ostream<CharT>& stream) -> std::basic_ostream<CharT>& {
    return stream << termcolor::red << label_padding << "error" << termcolor::reset;
  }

  template <typename CharT>
  static auto update_label(std::basic_ostream<CharT>& stream) -> std::basic_ostream<CharT>& {
    return stream << termcolor::green << label_padding << "update" << termcolor::reset;
  }

  template <typename CharT>
  static auto info_label(std::basic_ostream<CharT>& stream) -> std::basic_ostream<CharT>& {
    return stream << termcolor::blue << label_padding << "info" << termcolor::reset;
  }
};