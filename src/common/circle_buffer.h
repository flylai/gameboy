#pragma once

#include <vector>

#include "common/type.h"

namespace gb {

template<class T>
class CircleBuffer {
public:
  explicit CircleBuffer(i32 capacity) : capacity_(capacity + 1) { buffer_.resize(capacity_); }

  void push(T value) {
    buffer_[tail_++] = value;
    tail_ %= capacity_;
    if (tail_ == head_) {
      head_ = (head_ + 1) % capacity_;
    }
  }

  T pop() {
    if (empty()) {
      return 0;
    }
    T ret = buffer_[head_++];
    head_ %= capacity_;
    return ret;
  }

  void popN(u32 n) {
    if (n > size()) {
      head_ = tail_ = 0;
      return;
    }
    head_ = (head_ + n) % capacity_;
  }

  T front() const {
    if (empty()) {
      return 0;
    }
    return buffer_[head_ % capacity_];
  }

  T back() const {
    if (empty()) {
      return 0;
    }
    return buffer_[(tail_ - 1 + capacity_) % capacity_];
  }

  T operator[](u32 index) const { return buffer_[(head_ + index) % capacity_]; }

  void clear() { head_ = tail_ = 0; }

  u32 size() const { return (tail_ + capacity_ - head_) % capacity_; }

  bool empty() const { return tail_ == head_; }

  bool full() const { return ((tail_ + 1) % capacity_) == head_; }

private:
  CircleBuffer() = delete;

  i32 head_{};
  i32 tail_{};
  i32 capacity_{};
  std::vector<T> buffer_;
};

} // namespace gb