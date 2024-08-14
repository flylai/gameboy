#pragma once

#include "machine/memory/memory_accessor.h"

namespace gb {

class APU;

class APUGlobalRegister : public Memory<0xFF24, 0xFF26> {
public:
  explicit APUGlobalRegister(APU* apu) : apu_(apu) {}

  bool audioEnable() const { return getBitN(get(0xFF26), 7); }

  bool leftEnable(u8 channel_index) const {
    GB_ASSERT(channel_index >= 1 && channel_index <= 4);
    return getBitN(get(0xff25), channel_index + 3);
  }

  bool rightEnable(u8 channel_index) const {
    GB_ASSERT(channel_index >= 1 && channel_index <= 4);
    return getBitN(get(0xff25), channel_index - 1);
  }

  void set(gb::u16 addr, gb::u8 val) override {
    if (addr == 0xFF26) {
      Memory::set(addr, val & 0xf0);
    } else {
      Memory::set(addr, val);
    }
  }

  u8 get(u16 addr) const override;

  u8& apuDIV() { return apu_div_; }

private:
  APU* apu_{};

  // a.k.a frame sequencer
  u8 apu_div_{};
};

} // namespace gb
