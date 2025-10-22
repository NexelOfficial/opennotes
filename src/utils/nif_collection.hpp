#pragma once
#include <domino/global.h>
#include <domino/nif.h>

#include <string>
#include <vector>

struct NIFData {
  USHORT type;
  std::vector<USHORT> buffer;
};

struct NIFEntry {
  NOTEID id;
  std::vector<NIFData> data{};
};

class NIFCollection {
 public:
  NIFCollection(DHANDLE db_handle, std::string view_name);
  ~NIFCollection();

  [[nodiscard]] auto is_valid() const -> bool { return this->handle != NULLHANDLE; }
  [[nodiscard]] auto get_handle() const -> HCOLLECTION { return this->handle; }

  [[nodiscard]] auto read_entries(
      COLLECTIONPOSITION* pos, DWORD return_count = 0xFFFFFFFF,
      DWORD read_mask = READ_MASK_NOTEID | READ_MASK_SUMMARYVALUES) const -> std::vector<NIFEntry>;

 private:
  HCOLLECTION handle = NULLHANDLE;
};