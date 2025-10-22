#pragma once
#include <domino/global.h>
#include <domino/nif.h>

#include <string>

struct NIFEntries {
  DWORD length;
  DHANDLE handle;
};

class NIFCollection {
 public:
  NIFCollection(DHANDLE db_handle, std::string view_name);
  ~NIFCollection();

  [[nodiscard]] auto is_valid() const -> bool { return this->handle != NULLHANDLE; }
  [[nodiscard]] auto get_handle() const -> HCOLLECTION { return this->handle; }

  [[nodiscard]] auto read_entries(COLLECTIONPOSITION* pos,
                                  DWORD read_mask = READ_MASK_NOTEID | READ_MASK_SUMMARYVALUES,
                                  DWORD return_count = 10) const -> NIFEntries;

 private:
  HCOLLECTION handle = NULLHANDLE;
};