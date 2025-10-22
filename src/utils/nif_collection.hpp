#pragma once
#include <string>

#include <domino/global.h>
#include <domino/nif.h>

class NIFCollection {
 public:
  NIFCollection(DHANDLE db_handle, std::string view_name);
  ~NIFCollection();

  [[nodiscard]] auto is_valid() const -> bool { return this->handle != NULLHANDLE; }
  [[nodiscard]] auto get_handle() const -> HCOLLECTION { return this->handle; }

 private:
  HCOLLECTION handle = NULLHANDLE;
};