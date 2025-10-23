#pragma once
#include <domino/global.h>
#include <domino/osmem.h>

#include <vector>

class OSObject {
 public:
  OSObject() = delete;
  OSObject(DHANDLE handle)
      : object_ptr(reinterpret_cast<BYTE*>(OSLockObject(handle))), handle(handle) {}
  OSObject(std::vector<USHORT> handle) : object_ptr(reinterpret_cast<BYTE*>(handle.data())) {}
  ~OSObject() { this->unlock_and_free(); }

  template <typename T>
  [[nodiscard]] auto get() -> T {
    auto res = *this->get_raw<T*>();
    this->inc(sizeof(T));
    return res;
  }

  template <typename T>
  [[nodiscard]] auto get(WORD len) -> std::vector<T> {
    std::vector<T> item_buffer(len);
    memcpy(item_buffer.data(), this->get_raw<T*>(), len);
    this->inc(len);
    return item_buffer;
  }

  template <typename T>
  [[nodiscard]] auto get_raw() const -> T {
    return reinterpret_cast<T>(this->object_ptr);
  }

  void inc(size_t amount) { object_ptr += amount; }
  void mov(BYTE* position) { object_ptr = position; }

  auto unlock_and_free() -> bool { return this->unlock() && this->free(); }
  auto unlock() -> bool { return handle ? OSUnlockObject(this->handle) : true; }
  auto free() -> bool { return handle ? OSMemFree(this->handle) : true; }

 private:
  BYTE* object_ptr = nullptr;
  DHANDLE handle = NULLHANDLE;
};
