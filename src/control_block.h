#pragma once

#include <cstddef>
#include <memory>
#include <new>

namespace detail {

struct control_block_base {
  size_t strong_ref_cnt{1};
  size_t weak_ref_cnt{};

  virtual void clear() = 0;
  virtual ~control_block_base() = default;

  void check_lifetime();
  void dec_strong();
  void dec_weak();
};

template <typename T, typename Deleter = std::default_delete<T>>
struct regular_control_block : control_block_base {
  T* ptr;
  [[no_unique_address]] Deleter deleter;

  regular_control_block(T* ptr, Deleter deleter) : ptr(ptr), deleter(std::move(deleter)) {}

  void clear() override {
    deleter(ptr);
  }
};

template <typename T>
struct inplace_control_block : control_block_base {
  template <typename... Args>
  explicit inplace_control_block(Args&&... args) {
    new (&data) T(std::forward<Args>(args)...);
  }

  alignas(T) std::byte data[sizeof(T)];

  T* get() noexcept {
    return std::launder(reinterpret_cast<T*>(data));
  }

  void clear() override {
    get()->~T();
  }
};

} // namespace detail
