#pragma once

#include "channel.h"
#include "common/logger.h"
#include "common/type.h"

namespace gb {

class Channel3 : public Channel {
public:
  Channel3() : Channel(256) {}

  void tick() override {
    if (period_timer_ == 0) {
      period_timer_  = (2048 - period()) << 1;
      wave_position_ = (wave_position_ + 1) & 0x1f;
    }
    period_timer_--;
  }

  void trigger() override {}

  bool enable() const override { return channel_enable_ & dacEnable(); }

  i16 output() const override {
    if (!dacEnable() || outputLevel() == 0) {
      return 0;
    }
    u8 wave = wave_pattern_ram_[wave_position_ << 1];
    if (wave_position_ & 1) {
      wave &= 0x0f;
    } else {
      wave >>= 4;
    }

    wave >>= outputLevel() - 1;
    return wave;
  }

  u8 get(u16 addr) const override {
    GB_ASSERT((addr >= 0xff1a && addr <= 0xff1e) || (addr >= 0xff30 && addr <= 0xff3f));
    switch (addr) {
      case 0xff1a:
        return nr30_;
      case 0xff1b:
        return length_timer_.get(addr);
      case 0xff1c:
        return nr32_;
      case 0xff1d:
        return nrx3_;
      case 0xff1e:
        return nrx4_;
      case 0xff30 ... 0xff3f:
        return wave_pattern_ram_[addr - 0xff30];
    }
    GB_UNREACHABLE();
  }

  void set(u16 addr, u8 val) override {
    GB_ASSERT((addr >= 0xff1a && addr <= 0xff1e) || (addr >= 0xff30 && addr <= 0xff3f));
    switch (addr) {
      case 0xff1a:
        nr30_ = val;
        break;
      case 0xff1b:
        length_timer_.set(addr, val);
        break;
      case 0xff1c:
        nr32_ = val;
        break;
      case 0xff1d:
        nrx3_ = val;
        break;
      case 0xff1e:
        nrx4_ = val;
        length_timer_.setNRx4Event(val);
        if (getBitN(val, 7)) {
          trigger();
        }
        break;
      case 0xff30 ... 0xff3f:
        // wave pattern ram
        wave_pattern_ram_[addr - 0xff30] = val;
        break;
    }
  }

  bool dacEnable() const { return getBitN(nr30_, 7); }

  u8 outputLevel() const { return (nr32_ >> 5) & 0x3; }

  LengthTimer& lengthTimer() { return length_timer_; }


private:
  u8 nr30_{}; // 0xff1a
  u8 nr32_{}; // 0xff1c

  u8 wave_pattern_ram_[16]{};

  u8 wave_position_{};
};

} // namespace gb
