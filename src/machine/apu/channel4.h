#pragma once

#include "channel.h"
#include "common/logger.h"
#include "length_timer.h"
#include "volume_envelope.h"

namespace gb {

class Channel4 : public Channel {
public:
  using Channel::Channel;

  void tick() override {
    if (--period_timer_ == 0) {
      u8 bit0 = lfsr_ & 1;
      u8 bit1 = (lfsr_ >> 1) & 1;
      u8 res  = bit0 ^ bit1;
      lfsr_ >>= 1;

      lfsr_ = clearBitN(lfsr_, 15);
      lfsr_ |= res << 14;

      if (lsfrWidth() == 7) {
        lfsr_ = clearBitN(lfsr_, 7);
        lfsr_ = (lfsr_ & 0x8000) >> 8;
      }

      period_timer_ = divisors_[clockDivisor()] << clockShift();
    }
  }

  i16 output() const override {
    if (enable() && (lfsr_ & 1)) {
      return envelope_.currentVolume();
    }
    return 0;
  }

  void trigger() override {
    envelope_.trigger();
    period_timer_   = divisors_[clockDivisor()] << clockShift();
    lfsr_           = rand();
    channel_enable_ = true;
  }

  bool enable() const override { return channel_enable_ && dacEnable() && !lengthTimer_.expiring(); }

  bool dacEnable() const override { return envelope_.dacEnable(); }

  u8 get(u16 addr) const override {
    switch (addr) {
      case 0xff20:
        return lengthTimer_.get(addr);
      case 0xff21:
        return envelope_.get(addr);
      case 0xff22:
        return nrx3_;
      case 0xff23:
        return nrx4_;
      default:
        GB_UNREACHABLE();
    }
  }

  void set(u16 addr, u8 val) override {
    switch (addr) {
      case 0xff20:
        lengthTimer_.set(addr, val);
        break;
      case 0xff21:
        envelope_.set(addr, val);
        break;
      case 0xff22:
        nrx3_ = val;
        break;
      case 0xff23:
        nrx4_ = val;
        length_timer_.setNRx4Event(val);

        if (getBitN(val, 7)) {
          trigger();
        }
        break;
      default:
        GB_UNREACHABLE();
    }
  }

  u8 clockShift() const { return nrx3_ >> 4; }

  u8 lsfrWidth() const { return getBitN(nrx3_, 3) ? 7 : 15; }

  u8 clockDivisor() const { return nrx3_ & 0x7; }

  bool lengthEnable() const { return getBitN(nrx4_, 6); }

  LengthTimer& lengthTimer() { return lengthTimer_; }

  Envelope& envelope() { return envelope_; }

private:
  static constexpr u8 divisors_[] = {8, 16, 32, 48, 64, 80, 96, 112};

  LengthTimer lengthTimer_; // nr41 0xff20
  Envelope envelope_;       // nr42 0xff21 lfsr

  u16 lfsr_;
};

} // namespace gb
