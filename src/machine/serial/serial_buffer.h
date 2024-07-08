#pragma once

#include <functional>
#include <vector>

#include "common/type.h"

namespace gb {
class SerialBuffer {
public:
  using SerialObserver = std::function<void(const std::vector<u8> &)>;
  using SerialUpdate   = std::function<void(u8)>;

  void push(u8 data) {
    buffer_.push_back(data);
    if (observer_) {
      observer_(buffer_);
    }
    if (update_) {
      update_(data);
    }
  }

  void serialObserver(const SerialObserver &observer) { observer_ = observer; }

  void serialUpdate(const SerialUpdate &update) { update_ = update; }

  const std::vector<u8> &buffer() const { return buffer_; }


private:
  SerialObserver observer_{};
  SerialUpdate update_{};
  std::vector<u8> buffer_;
};
} // namespace gb
