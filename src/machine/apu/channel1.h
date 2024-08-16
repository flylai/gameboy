#pragma once
#include "channel2.h"

namespace gb {

class Channel1 : public Channel2 {
public:
  enum class Direction {
    kDECREASE = 0,
    kINCREASE = 1,
  };

  void trigger() override {
    Channel2::trigger();

    // sweep
    shadow_frequency_ = frequency_;
    sweep_timer_      = sweepPace() == 0 ? 8 : sweepPace();
    sweep_enable_     = sweepPace() != 0 || sweepIndividualStep() != 0;

    if (sweepIndividualStep() != 0) {
      calculateFrequency();
    }
  }

  u16 period() const override { return frequency_; }

  u16 calculateFrequency() {
    u16 new_freq = shadow_frequency_ >> sweepIndividualStep();
    if (sweepDirection() == Direction::kDECREASE) {
      new_freq = shadow_frequency_ - new_freq;
    } else {
      new_freq = shadow_frequency_ + new_freq;
    }
    if (new_freq > 2047) {
      channel_enable_ = false;
    }
    return new_freq;
  }

  void tickSweep() {
    if (sweep_timer_ > 0) {
      sweep_timer_--;
    }

    if (sweep_timer_ == 0) {
      sweep_timer_ = sweepPace() == 0 ? 8 : sweepPace();

      if (sweep_enable_ && sweepPace() != 0) {
        // calculate new frequency
        u16 new_freq = calculateFrequency();
        if (new_freq > 2047 && sweepIndividualStep() != 0) {
          frequency_        = new_freq;
          shadow_frequency_ = new_freq;
          calculateFrequency();
        }
      }
    }
  }

  u8 get(u16 addr) const override {
    if (addr == 0xff10) {
      return nr10_;
    } else {
      return Channel2::get(addr + 5);
    }
  }

  void set(u16 addr, u8 val) override {
    if (addr == 0xff10) {
      nr10_ = val;
    } else {
      Channel2::set(addr + 5, val);
      if (inOr(addr, 0xff13, 0xff14)) {
        frequency_ = Channel2::period();
      }
      if (addr == 0xff14 && getBitN(val, 7) && dacEnable()) {
        trigger();
      }
    }
  }

  u8 sweepPace() const { return (nr10_ >> 4) & 0x7; }

  Direction sweepDirection() const { return static_cast<Direction>(getBitN(nr10_, 3)); }

  u8 sweepIndividualStep() const { return nr10_ & 0x7; }

private:
  u8 nr10_{}; // sweep

  u16 frequency_{};

  bool sweep_enable_{};
  u8 sweep_timer_{};
  u16 shadow_frequency_{};
};

} // namespace gb
