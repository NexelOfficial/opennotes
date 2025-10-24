#pragma once
#include <string>

#include <domino/global.h>

#include "os.hpp"

class Formula {
 public:
  Formula(std::string formula_str);
  ~Formula() { this->formula_obj->unlock_and_free(); }

  [[nodiscard]] auto evaluate(DHANDLE note_handle) -> std::string;

 private:
  OSObject *formula_obj;
  USHORT formula_size = NULL;

  DHANDLE compute_handle = NULLHANDLE;

  auto compute_start() -> STATUS;
};