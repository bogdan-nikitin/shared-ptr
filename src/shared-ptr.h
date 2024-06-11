#pragma once

#include "base_ptr.h"
#include "control_block.h"

#include <cstddef>
#include <memory>
#include <utility>

template <typename T>
class shared_ptr : private detail::base_ptr<T> {
  template <typename>
  friend class weak_ptr;

  template <typename>
  friend class shared_ptr;

private:
  using base = detail::base_ptr<T>;

  shared_ptr(detail::control_block_base* cb, T* ptr) : base(cb, ptr) {}

  void inc_ref() noexcept {
    if (this->cb) {
      ++this->cb->strong_ref_cnt;
    }
  }

  void dec_ref() noexcept {
    if (this->cb) {
      this->cb->dec_strong();
    }
  }

  template <typename Y>
  shared_ptr(const detail::base_ptr<Y>& other, T* ptr) : base(other, ptr) {
    inc_ref();
  }

  template <typename Y>
  shared_ptr(const detail::base_ptr<Y>& other) : shared_ptr(other, other.ptr) {}

public:
  shared_ptr() noexcept = default;

  shared_ptr(std::nullptr_t) noexcept : shared_ptr() {}

  template <typename Y>
  explicit shared_ptr(Y* ptr) : shared_ptr(ptr, std::default_delete<Y>()) {}

  template <typename Y, typename Deleter>
  shared_ptr(Y* ptr, Deleter deleter) {
    try {
      this->cb = new detail::regular_control_block<Y, Deleter>(ptr, std::move(deleter));
      this->ptr = ptr;
    } catch (...) {
      deleter(ptr);
      throw;
    }
  }

  template <typename Y>
  shared_ptr(const shared_ptr<Y>& other, T* ptr) noexcept : shared_ptr(other.as_base(), ptr) {}

  template <typename Y>
  shared_ptr(shared_ptr<Y>&& other, T* ptr) noexcept : base(std::move(other), ptr) {}

  shared_ptr(const shared_ptr& other) noexcept : shared_ptr(other.as_base()) {}

  template <typename Y>
  shared_ptr(const shared_ptr<Y>& other) noexcept : shared_ptr(other.as_base()) {}

  shared_ptr(shared_ptr&& other) noexcept : base(std::move(other)) {}

  template <typename Y>
  shared_ptr(shared_ptr<Y>&& other) noexcept : base(std::move(other)) {}

  shared_ptr& operator=(const shared_ptr& other) noexcept {
    operator=<T>(other);
    return *this;
  }

  template <typename Y>
  shared_ptr& operator=(const shared_ptr<Y>& other) noexcept {
    if (this->cb == other.cb) {
      this->ptr = other.ptr;
      return *this;
    }
    dec_ref();
    this->copy_members(other);
    inc_ref();
    return *this;
  }

  shared_ptr& operator=(shared_ptr&& other) noexcept {
    operator=<T>(std::move(other));
    return *this;
  }

  template <typename Y>
  shared_ptr& operator=(shared_ptr<Y>&& other) noexcept {
    if (static_cast<void*>(this) == static_cast<void*>(&other)) {
      return *this;
    }
    dec_ref();
    this->copy_members(other);
    other.null();
    return *this;
  }

  T* get() const noexcept {
    return this->ptr;
  }

  operator bool() const noexcept {
    return get();
  }

  T& operator*() const noexcept {
    return *get();
  }

  T* operator->() const noexcept {
    return get();
  }

  std::size_t use_count() const noexcept {
    return this->cb ? this->cb->strong_ref_cnt : 0;
  }

  void reset() noexcept {
    shared_ptr<T>().swap(*this);
  }

  template <typename Y>
  void reset(Y* new_ptr) {
    reset(new_ptr, std::default_delete<Y>());
  }

  template <typename Y, typename Deleter>
  void reset(Y* new_ptr, Deleter deleter) {
    shared_ptr(new_ptr, std::move(deleter)).swap(*this);
  }

  ~shared_ptr() {
    dec_ref();
  }

  template <typename Y, typename U>
  friend bool operator==(const shared_ptr<Y>& lhs, const shared_ptr<U>& rhs) noexcept;

  template <typename Y, typename U>
  friend bool operator!=(const shared_ptr<Y>& lhs, const shared_ptr<U>& rhs) noexcept;

  template <typename Y>
  friend bool operator==(const shared_ptr<Y>& lhs, std::nullptr_t) noexcept;

  template <typename Y>
  friend bool operator!=(const shared_ptr<Y>& lhs, std::nullptr_t) noexcept;

  template <typename Y>
  friend bool operator==(std::nullptr_t, const shared_ptr<Y>& rhs) noexcept;

  template <typename Y>
  friend bool operator!=(std::nullptr_t, const shared_ptr<Y>& rhs) noexcept;

  template <typename U, typename... Args>
  friend shared_ptr<U> make_shared(Args&&... args);
};

template <typename T>
class weak_ptr : detail::base_ptr<T> {
  template <typename>
  friend class weak_ptr;

private:
  using base = detail::base_ptr<T>;

  void inc_ref() noexcept {
    if (this->cb) {
      ++this->cb->weak_ref_cnt;
    }
  }

  void dec_ref() noexcept {
    if (this->cb) {
      this->cb->dec_weak();
    }
  }

  template <typename Y>
  weak_ptr& operator=(const detail::base_ptr<Y>& other) noexcept {
    if (this->cb == other.cb) {
      this->ptr = other.ptr;
      return *this;
    }
    dec_ref();
    this->copy_members(other);
    inc_ref();
    return *this;
  }

public:
  weak_ptr() noexcept = default;

  template <typename Y>
  weak_ptr(const shared_ptr<Y>& other) noexcept : base(other) {
    inc_ref();
  }

  weak_ptr(const weak_ptr& other) noexcept : base(other) {
    inc_ref();
  }

  template <typename Y>
  weak_ptr(const weak_ptr<Y>& other) noexcept : base(other) {
    inc_ref();
  }

  weak_ptr(weak_ptr&& other) noexcept : base(std::move(other)) {}

  template <typename Y>
  weak_ptr(weak_ptr<Y>&& other) noexcept : base(std::move(other)) {}

  template <typename Y>
  weak_ptr& operator=(const shared_ptr<Y>& other) noexcept {
    operator=<Y>(other.as_base());
    return *this;
  }

  weak_ptr& operator=(const weak_ptr& other) noexcept {
    operator=<T>(other.as_base());
    return *this;
  }

  template <typename Y>
  weak_ptr& operator=(const weak_ptr<Y>& other) noexcept {
    operator=<Y>(other.as_base());
    return *this;
  }

  weak_ptr& operator=(weak_ptr&& other) noexcept {
    operator=<T>(std::move(other));
    return *this;
  }

  template <typename Y>
  weak_ptr& operator=(weak_ptr<Y>&& other) noexcept {
    if (static_cast<void*>(this) == static_cast<void*>(&other)) {
      return *this;
    }
    dec_ref();
    this->copy_members(other);
    other.null();
    return *this;
  }

  shared_ptr<T> lock() const noexcept {
    return this->cb && this->cb->strong_ref_cnt ? shared_ptr<T>{*this} : shared_ptr<T>{};
  }

  void reset() {
    weak_ptr<T>().swap(*this);
  }

  ~weak_ptr() {
    dec_ref();
  }
};

template <typename T, typename... Args>
shared_ptr<T> make_shared(Args&&... args) {
  auto cb = new detail::inplace_control_block<T>(std::forward<Args>(args)...);
  return shared_ptr<T>(static_cast<detail::control_block_base*>(cb), cb->get());
}

template <typename Y, typename U>
bool operator==(const shared_ptr<Y>& lhs, const shared_ptr<U>& rhs) noexcept {
  return lhs.get() == rhs.get();
}

template <typename Y, typename U>
bool operator!=(const shared_ptr<Y>& lhs, const shared_ptr<U>& rhs) noexcept {
  return !(lhs == rhs);
}

template <typename Y>
bool operator==(const shared_ptr<Y>& lhs, std::nullptr_t) noexcept {
  return !lhs;
}

template <typename Y>
bool operator!=(const shared_ptr<Y>& lhs, std::nullptr_t) noexcept {
  return lhs;
}

template <typename Y>
bool operator==(std::nullptr_t, const shared_ptr<Y>& rhs) noexcept {
  return !rhs;
}

template <typename Y>
bool operator!=(std::nullptr_t, const shared_ptr<Y>& rhs) noexcept {
  return rhs;
}
