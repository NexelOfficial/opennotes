#pragma once
#include <string>

#include <domino/dirctx.h>
#include <domino/osmisc.h>

class Log {
 public:
  static void error(std::string message);
  static auto error(STATUS code, std::string prefix) -> STATUS;
};