#pragma once
#include <domino/global.h>
#include <domino/osmem.h>

class OSObject {
 public:
  OSObject(DHANDLE handle)
      : object_ptr(reinterpret_cast<char*>(OSLockObject(handle))), handle(handle) {}

  template <typename T>
  [[nodiscard]] auto get() -> T {
    return *this->get_raw<T*>();
  }

  template <typename T>
  [[nodiscard]] auto get_raw() -> T {
    return reinterpret_cast<T>(this->object_ptr);
  }

  void inc(size_t amount) { object_ptr += amount; }

  auto unlock_and_free() -> bool { return this->unlock() && this->free(); }
  auto unlock() -> bool { return OSUnlockObject(this->handle); }
  auto free() -> bool { return OSMemFree(this->handle); }

 private:
  char* object_ptr = nullptr;
  DHANDLE handle = NULLHANDLE;
};
