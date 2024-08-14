#pragma once
#include <cmath>

#include "channel.h"
#include "length_timer.h"
#include "volume_envelope.h"

namespace gb {

class Channel2 : public Channel {
public:
  using Channel::Channel;

  void tick() override {
    if (period_timer_ == 0) {
      period_timer_       = (2048 - period()) << 2;
      wave_duty_position_ = (wave_duty_position_ + 1) & 0x7;
    }
    period_timer_--;
  }

  void trigger() override {
    period_timer_ = (2048 - period()) << 2;
    length_timer_.trigger();
    envelope_.trigger();
    channel_enable_ = envelope_.dacEnable();
  }

  i16 output() const override {
    if (!enable()) {
      return 0;
    } else {
      return (wave_duty_[length_timer_.waveDuty()][wave_duty_position_] * envelope_.currentVolume());
    }
  }

  bool enable() const override {
    return channel_enable_ && envelope_.dacEnable() && !length_timer_.expiring();
  }

  u8 get(u16 addr) const override {
    if (addr == 0xff16) {
      return length_timer_.get(addr);
    } else if (addr == 0xff17) {
      return envelope_.get(addr);
    } else if (addr == 0xff18) {
      // write only
      return 0xff;
    } else if (addr == 0xff19) {
      return length_timer_.enable() << 6;
    }
    return 0;
  }

  void set(u16 addr, u8 val) override {
    if (addr == 0xff16) {
      length_timer_.set(addr, val);
    } else if (addr == 0xff17) {
      envelope_.set(addr, val);
      channel_enable_ &= envelope_.dacEnable();
    } else if (addr == 0xff18) {
      nrx3_ = val;
    } else if (addr == 0xff19) {
      nrx4_ = val;
      length_timer_.setNRx4Event(val);
      if (getBitN(val, 7)) {
        trigger();
        if (envelope_.dacEnable()) {
          channel_enable_ = true;
        }
      }
    }
  }

  LengthTimer& lengthTimer() { return length_timer_; }

  Envelope& envelope() { return envelope_; }

protected:
  // https://gbdev.io/pandocs/Audio_Registers.html#ff11--nr11-channel-1-length-timer--duty-cycle
  static constexpr u8 wave_duty_[][8] = {
          {1, 1, 1, 1, 1, 1, 1, 0}, // 12.5%
          {0, 1, 1, 1, 1, 1, 1, 0}, // 25%
          {0, 1, 1, 1, 1, 0, 0, 0}, // 50%
          {1, 0, 0, 0, 0, 0, 0, 1}, // 75%
  };

  u8 wave_duty_position_{};

  Envelope envelope_; // nrx2
};

} // namespace gb
