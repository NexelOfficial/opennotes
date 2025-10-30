#pragma once
#include <domino/global.h>
#include <domino/nif.h>

#include <string>
#include <vector>

struct NIFColumn {
  USHORT type;
  std::vector<USHORT> buffer;
};

struct NIFEntry {
  NOTEID id;
  std::vector<NIFColumn> columns{};
};

class View {
 public:
  View(DHANDLE db_handle, std::string view_name);
  ~View();

  [[nodiscard]] auto is_valid() const -> bool { return this->handle != NULLHANDLE; }
  [[nodiscard]] auto get_handle() const -> HCOLLECTION { return this->handle; }

  [[nodiscard]] auto get_entries(DWORD return_count = 0xFFFFFFFF) const -> std::vector<NIFEntry>;

 private:
  HCOLLECTION handle = NULLHANDLE;
};