#include "apu.h"

#include <unistd.h>

#include <cmath>

namespace gb {

void APU::tick() {
  // bug here
  // it triggers by DIV register of timer when bit 4 goes from 1 to 0
  // we can set DIV to 0 to make it trigger faster,
  // so maybe we need to read DIV register in the future
  if (++t_cycle_counter_ == 8192) {
    t_cycle_counter_       = 0;


    // apu_div starts from 0, and self add,
    // so we need to minus 1 to make it start from 0
    bool trigger_envelope  = inOr(apu_reg_.apuDIV(), 7);
    bool trigger_length    = inOr(apu_reg_.apuDIV(), 0, 2, 4, 6);
    bool trigger_ch1_sweep = inOr(apu_reg_.apuDIV(), 2, 6);

    apu_reg_.apuDIV()      = (apu_reg_.apuDIV() + 1) & 0x7;


    if (trigger_envelope) {
      channel1_.envelope().tick();
      channel2_.envelope().tick();
      channel4_.envelope().tick();
    }

    if (trigger_length) {
      channel1_.lengthTimer().tick();
      channel2_.lengthTimer().tick();
      channel3_.lengthTimer().tick();
      channel4_.lengthTimer().tick();
    }

    if (trigger_ch1_sweep) {
      channel1_.tickSweep();
    }
  }
  //

  channel1_.tick();
  channel2_.tick();
  channel3_.tick();
  channel4_.tick();

  if ((++sample_buffer_counter_) == 87) {
    sample_buffer_counter_ = 0;
    std::lock_guard lock(sample_buffer_mutex_);
    i32 left{};
    i32 right{};

#define OUTPUT(CHANNEL, CHANNEL_IDX)             \
  if (apu_reg_.CHANNEL##Enable(CHANNEL_IDX)) {   \
    CHANNEL += channel##CHANNEL_IDX##_.output(); \
  }

    OUTPUT(left, 1);
    OUTPUT(left, 2);
    OUTPUT(left, 3);
    OUTPUT(left, 4);

    OUTPUT(right, 1);
    OUTPUT(right, 2);
    OUTPUT(right, 3);
    OUTPUT(right, 4);

    left  = (left - 32) << 10;
    right = (right - 32) << 10;

    sample_buffer_.push(left);
    sample_buffer_.push(right);
  }
#undef OUTPUT
}

void APU::audioDataCallback(ma_device *device, void *output, const void *input, ma_uint32 frame_count) {
  std::lock_guard lock(sample_buffer_mutex_);

  if (sample_buffer_.size() << 1 < frame_count) {
    return;
  }

  i16 *outputI16 = (i16 *) output;

  for (u32 i = 0; i < frame_count; i++) {
    outputI16[i * 2]     = sample_buffer_.pop();
    outputI16[i * 2 + 1] = sample_buffer_.pop();
  }
}

u8 APU::get(u16 addr) const {
  switch (addr) {
    case 0xff10 ... 0xff14:
      return channel1_.get(addr);
    case 0xff16 ... 0xff19:
      return channel2_.get(addr);
    case 0xff1a ... 0xff1e:
      return channel3_.get(addr);
    case 0xff20 ... 0xff23:
      return channel4_.get(addr);
    case 0xff24 ... 0xff26:
      return apu_reg_.get(addr);
    case 0xff30 ... 0xff3f:
      return channel3_.get(addr);
    default:
      return 0xff;
  }
}

void APU::set(u16 addr, u8 val) {
  switch (addr) {
    case 0xff10 ... 0xff14:
      channel1_.set(addr, val);
      break;
    case 0xff16 ... 0xff19:
      channel2_.set(addr, val);
      break;
    case 0xff1a ... 0xff1e:
      channel3_.set(addr, val);
      break;
    case 0xff20 ... 0xff23:
      channel4_.set(addr, val);
      break;
    case 0xff24 ... 0xff26:
      apu_reg_.set(addr, val);
      break;
    case 0xff30 ... 0xff3f:
      channel3_.set(addr, val);
      break;
    case 0xff15:
    case 0xff1f:
      // some ROM access this register, WHY??
      break;
    default:
      GB_UNREACHABLE();
  }
}

} // namespace gb
