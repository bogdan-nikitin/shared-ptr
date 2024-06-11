#pragma once

#include "control_block.h"

#include <utility>

template <typename T>
class shared_ptr;

template <typename T>
class weak_ptr;

namespace detail {
template <typename T>
struct base_ptr {
  template <typename>
  friend class ::shared_ptr;

  template <typename>
  friend class ::weak_ptr;

  template <typename>
  friend struct base_ptr;

private:
  template <typename Y>
  void move(base_ptr<Y>&& other) {
    copy_members(other);
    other.null();
  }

protected:
  template <typename Y>
  void copy_members(const base_ptr<Y>& other) noexcept {
    cb = other.cb;
    ptr = other.ptr;
  }

  base_ptr() noexcept = default;

  template <typename Y>
  base_ptr(const base_ptr<Y>& other) noexcept {
    copy_members(other);
  }

  base_ptr(const base_ptr& other) noexcept {
    copy_members(other);
  }

  template <typename Y>
  base_ptr(const base_ptr<Y>& other, T* ptr) noexcept : cb(other.cb),
                                                        ptr(ptr) {}

  template <typename Y>
  base_ptr(base_ptr<Y>&& other) {
    move(std::move(other));
  }

  base_ptr(base_ptr&& other) {
    move(std::move(other));
  }

  base_ptr(control_block_base* cb, T* ptr) : cb(cb), ptr(ptr) {}

  template <typename Y>
  base_ptr(base_ptr<Y>&& other, T* ptr) noexcept : base_ptr(other.cb, ptr) {
    other.null();
  }

  void null() noexcept {
    cb = nullptr;
    ptr = nullptr;
  }

  template <typename Y>
  void swap(base_ptr<Y>& other) noexcept {
    std::swap(cb, other.cb);
    std::swap(ptr, other.ptr);
  }

  base_ptr& as_base() {
    return *this;
  }

  const base_ptr& as_base() const {
    return *this;
  }

  control_block_base* cb = nullptr;
  T* ptr = nullptr;
};

} // namespace detail
