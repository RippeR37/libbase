#pragma once

#include <atomic>
#include <cstddef>
#include <memory>
#include <utility>

#include "base/sequence_checker.h"

namespace base {

template <typename T>
class WeakPtr;
template <typename T>
class WeakPtrFactory;

namespace detail {
struct WeakPtrControlBlock {
  WeakPtrControlBlock() : weak_count(0) {
    DETACH_FROM_SEQUENCE(sequence_checker);
  }

  std::atomic_size_t weak_count;
  SEQUENCE_CHECKER(sequence_checker);
};
}  // namespace detail

template <typename T>
class WeakPtr {
 public:
  WeakPtr() : ptr_(nullptr) {}
  explicit WeakPtr(std::nullptr_t) : WeakPtr() {}
  ~WeakPtr() { DecreaseWeakCount(); }

  template <typename U>
  WeakPtr(const WeakPtr<U>& other)
      : ptr_(other.ptr_), control_block_(other.control_block_) {
    IncreaseWeakCount();
  }

  template <typename U>
  WeakPtr(WeakPtr<U>&& other)
      : ptr_(std::exchange(other.ptr_, nullptr)),
        control_block_(std::exchange(other.control_block_, {})) {}

  template <typename U>
  WeakPtr& operator=(const WeakPtr<U>& other) {
    if (reinterpret_cast<uintptr_t>(this) !=
        reinterpret_cast<uintptr_t>(&other)) {
      DecreaseWeakCount();
      ptr_ = other.ptr_;
      control_block_ = other.control_block_;
      IncreaseWeakCount();
    }
    return *this;
  }

  template <typename U>
  WeakPtr& operator=(WeakPtr<U>&& other) {
    if (reinterpret_cast<uintptr_t>(this) !=
        reinterpret_cast<uintptr_t>(&other)) {
      DecreaseWeakCount();
      ptr_ = std::exchange(other.ptr_, nullptr);
      control_block_ = std::exchange(other.control_block_, {});
    }
    return *this;
  }

  T* operator->() const {
    DCHECK(Get());
    return Get();
  }

  T& operator*() const {
    DCHECK(Get());
    return *Get();
  }

  explicit operator bool() const { return (Get() != nullptr); }

  T* Get() const {
    if (auto control_block = control_block_.lock()) {
      DCHECK_CALLED_ON_VALID_SEQUENCE(control_block->sequence_checker);
      return ptr_;
    }
    return nullptr;
  }

  bool MaybeValid() const { return !control_block_.expired(); }

  bool WasInvalidated() const {
    if (auto control_block = control_block_.lock()) {
      DCHECK_CALLED_ON_VALID_SEQUENCE(control_block->sequence_checker);
      return false;
    }
    return !!ptr_;
  }

 private:
  friend class WeakPtrFactory<T>;
  template <typename U>
  friend class WeakPtr;

  WeakPtr(T* ptr, std::weak_ptr<detail::WeakPtrControlBlock> control_block)
      : ptr_(ptr), control_block_(std::move(control_block)) {
    IncreaseWeakCount();
  }

  void IncreaseWeakCount() {
    if (auto control_block = control_block_.lock()) {
      ++(control_block->weak_count);
    }
  }

  void DecreaseWeakCount() {
    if (auto control_block = control_block_.lock()) {
      --(control_block->weak_count);
    }
  }

  T* ptr_;
  std::weak_ptr<detail::WeakPtrControlBlock> control_block_;
};

template <typename T>
bool operator==(const WeakPtr<T>& weak_ptr, std::nullptr_t) {
  return (weak_ptr.Get() == nullptr);
}

template <typename T>
bool operator==(std::nullptr_t, const WeakPtr<T>& weak_ptr) {
  return (weak_ptr == nullptr);
}

template <typename T>
bool operator!=(const WeakPtr<T>& weak_ptr, std::nullptr_t) {
  return !(weak_ptr == nullptr);
}

template <typename T>
bool operator!=(std::nullptr_t, const WeakPtr<T>& weak_ptr) {
  return (weak_ptr != nullptr);
}

template <typename T>
class WeakPtrFactory {
 public:
  explicit WeakPtrFactory(T* ptr)
      : ptr_(ptr),
        control_block_(std::make_shared<detail::WeakPtrControlBlock>()) {
    DCHECK(ptr_);
  }

  WeakPtrFactory(const WeakPtrFactory&) = delete;
  WeakPtrFactory(WeakPtrFactory&&) = delete;
  WeakPtrFactory& operator=(const WeakPtrFactory&) = delete;
  WeakPtrFactory& operator=(WeakPtrFactory&&) = delete;

  WeakPtr<T> GetWeakPtr() const {
    if (!HasWeakPtrs()) {
      DETACH_FROM_SEQUENCE(control_block_->sequence_checker);
    }
    return WeakPtr<T>{ptr_, control_block_};
  }

  void InvalidateWeakPtrs() {
    DCHECK(control_block_);
    DCHECK_CALLED_ON_VALID_SEQUENCE(control_block_->sequence_checker);
    control_block_ = std::make_shared<detail::WeakPtrControlBlock>();
  }

  bool HasWeakPtrs() const {
    DCHECK(control_block_);
    return (control_block_->weak_count > 0);
  }

 protected:
  T* const ptr_;
  std::shared_ptr<detail::WeakPtrControlBlock> control_block_;
};

}  // namespace base
