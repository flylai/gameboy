#pragma once

#include "common/logger.h"
#include "common/type.h"
#include "machine/memory/memory_accessor.h"

// a lot of bug here...

namespace gb {

class LengthTimer : MemoryAccessor {
public:
  LengthTimer(u16 max_length = 64) : max_length_(max_length) {}

  void tick() {
    if (length_timer_ && enable_) {
      if (--length_timer_ == 0) {
        enable_ = 0;
      }
    }
  }

  void trigger() {
    if (expiring()) {
      length_timer_ = max_length_ /* - validBit()*/;
    }
  }

  u8 get(u16 addr) const override { return nrx1_; }

  void set(u16 addr, u8 val) override {
    nrx1_         = val;
    length_timer_ = max_length_ - initialLengthTimer();
  }

  void setNRx4Event(u8 nrx4) {
    enable_ = getBitN(nrx4, 6);
    if (getBitN(nrx4, 7)) {
      trigger();
    }

    if (length_timer_ == 0) {
      length_timer_ = max_length_;
    }
  }

  bool enable() const { return enable_; }

  bool expiring() const { return length_timer_ == 0; }

  u8 waveDuty() const { return nrx1_ >> 6; } // channel 1,2 only

  u8 initialLengthTimer() const { return nrx1_ & validBit(); }

  u8 nrx1() const { return nrx1_; }

private:
  u8 validBit() const { return max_length_ == 64 ? 0x3f : 0xff; }

  u8 nrx1_{};
  u8 enable_{};

  const u16 max_length_{64}; // 64, 256 for channel3 only
  u8 length_timer_{};
};

} // namespace gb
