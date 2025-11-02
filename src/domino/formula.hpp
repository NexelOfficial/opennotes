#pragma once
#include <domino/global.h>

#include <string>

#include "os.hpp"


class Formula {
 public:
  Formula(std::string formula_str);
  ~Formula() {
    this->formula_obj->unlock_and_free();
    delete formula_obj;
  }

  [[nodiscard]] auto evaluate(DHANDLE note_handle) -> std::string;

 private:
  OSObject *formula_obj;
  USHORT formula_size = NULL;

  DHANDLE compute_handle = NULLHANDLE;

  auto compute_start() -> STATUS;
};