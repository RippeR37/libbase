#pragma once

#include <atomic>
#include <cstddef>
#include <memory>
#include <utility>

namespace base {

template <typename T>
class WeakPtr;
template <typename T>
class WeakPtrFactory;

template <typename T>
class WeakPtr {
 public:
  WeakPtr() : ptr_(nullptr) {}
  WeakPtr(std::nullptr_t) : WeakPtr() {}
  ~WeakPtr() { DecreaseWeakCount(); }

  template <typename U>
  WeakPtr(const WeakPtr<U>& other)
      : ptr_(other.ptr_), weak_count_(other.weak_count_) {
    IncreaseWeakCount();
  }

  template <typename U>
  WeakPtr(WeakPtr<U>&& other)
      : ptr_(std::exchange(other.ptr_, nullptr)),
        weak_count_(std::exchange(other.weak_count_, {})) {}

  template <typename U>
  WeakPtr& operator=(const WeakPtr<U>& other) {
    if (reinterpret_cast<uintptr_t>(this) !=
        reinterpret_cast<uintptr_t>(&other)) {
      DecreaseWeakCount();
      ptr_ = other.ptr_;
      weak_count_ = other.weak_count_;
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
      weak_count_ = std::exchange(other.weak_count_, {});
    }
    return *this;
  }

  T* operator->() const {
    // DCHECK(Get());
    return Get();
  }

  T& operator*() const {
    // DCHECK(Get());
    return *Get();
  }

  explicit operator bool() const { return (Get() != nullptr); }

  T* Get() const {
    // DCHECK_CALLED_ON_VALID_SEQUENCE()
    if (auto weak_count = weak_count_.lock()) {
      return ptr_;
    }
    return nullptr;
  }

  bool MaybeValid() const { return !!(weak_count_.lock()); }

  bool WasInvalidated() const {
    // DCHECK_CALLED_ON_VALID_SEQUENCE
    return ptr_ && !weak_count_.lock();
  }

 private:
  friend class WeakPtrFactory<T>;
  template <typename U>
  friend class WeakPtr;

  WeakPtr(T* ptr, std::weak_ptr<std::atomic_size_t> weak_count)
      : ptr_(ptr), weak_count_(std::move(weak_count)) {
    IncreaseWeakCount();
  }

  void IncreaseWeakCount() {
    if (auto count = weak_count_.lock()) {
      ++(*count);
    }
  }

  void DecreaseWeakCount() {
    if (auto weak_count = weak_count_.lock()) {
      --(*weak_count);
    }
  }

  T* ptr_;
  std::weak_ptr<std::atomic_size_t> weak_count_;
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
      : ptr_(ptr), weak_count_(std::make_shared<std::atomic_size_t>(0)) {}

  WeakPtrFactory(const WeakPtrFactory&) = delete;
  WeakPtrFactory(WeakPtrFactory&&) = delete;
  WeakPtrFactory& operator=(const WeakPtrFactory&) = delete;
  WeakPtrFactory& operator=(WeakPtrFactory&&) = delete;

  WeakPtr<T> GetWeakPtr() const { return WeakPtr<T>{ptr_, weak_count_}; }

  void InvalidateWeakPtrs() {
    // DCHECK_CALLED_ON_VALID_SEQUENCE
    // DCHECK(ptr_);
    weak_count_ = std::make_shared<std::atomic_size_t>(0);
  }

  bool HasWeakPtrs() const {
    // DCHECK_CALLED_ON_VALID_SEQUENCE
    return weak_count_ && (*weak_count_ > 0);
  }

 protected:
  T* const ptr_;
  std::shared_ptr<std::atomic_size_t> weak_count_;
};

}  // namespace base
