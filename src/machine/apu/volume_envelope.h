#pragma once

#include "common/type.h"
#include "machine/memory/memory_accessor.h"

namespace gb {

class Envelope : MemoryAccessor {
public:
  enum class Direction {
    kDECREASE = 0,
    kINCREASE = 1,
  };

  u8 get(u16 addr) const override { return nrx2; }

  void set(u16 addr, u8 val) override { nrx2 = val; }

  u8 initialVolume() const { return nrx2 >> 4; }

  Direction direction() const { return static_cast<Direction>(getBitN(nrx2, 3)); }

  u8 sweepPace() const { return nrx2 & 0x7; }

  bool dacEnable() const { return nrx2 & 0xf8; }

  void tick() {
    // if (!dacEnable()) {
    //   return;
    // }

    if (sweepPace() == 0) {
      return;
    }

    if (period_timer_ > 0) {
      period_timer_--;
    }

    if (period_timer_ == 0) {
      period_timer_ = sweepPace() == 0 ? 8 : sweepPace();
    }

    if (direction() == Direction::kDECREASE && current_volume_ > 0) {
      current_volume_--;
    } else if (direction() == Direction::kINCREASE && current_volume_ < 0xf) {
      current_volume_++;
    } else {
      // disable envelope
      // do nothing
      return;
    }

    if (current_volume_ == 0) {
      current_volume_ = initialVolume();
    }
  }

  void trigger() {
    current_volume_ = initialVolume();
    period_timer_   = sweepPace() == 0 ? 8 : sweepPace();
  }

  u8 currentVolume() const { return period_timer_ > 0 ? current_volume_ : initialVolume(); }

private:
  u8 nrx2{};

  u8 period_timer_{};
  u8 current_volume_{};
};

} // namespace gb
