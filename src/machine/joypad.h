#pragma once
#include "machine/memory/memory_accessor.h"

namespace gb {
class MemoryBus;

class Joypad : public Memory<0xff00, 0xff00> {
public:
  Joypad() { ram_[0] = 0xff; }

#define DECL_KEY(NAME, TYPE, REG_BIT) void NAME(bool press);

  DECL_KEY(A, select, 0)
  DECL_KEY(B, select, 1)
  DECL_KEY(Select, select, 2)
  DECL_KEY(Start, select, 3)
  DECL_KEY(Right, direction, 0)
  DECL_KEY(Left, direction, 1)
  DECL_KEY(Up, direction, 2)
  DECL_KEY(Down, direction, 3)

  void set(gb::u16 addr, gb::u8 val) override { ram_[0] = (val & 0x30) | (ram_[0] & 0xcf); }

  u8 get(u16) const override {
    u8 bits45 = ram_[0] & 0x30;
    u8 ret    = ram_[0];
    if (bits45 == 0x10) {
      // select
      ret = bits45 | (select_ & 0xf) | 0xc0;
    } else if (bits45 == 0x20) {
      // direction
      ret = bits45 | (direction_ & 0xf) | 0xc0;
    } else {
      ret &= 0xff;
    }
    return ret;
  }

  void memoryBus(MemoryBus* memory_bus) { memory_bus_ = memory_bus; }

private:
  void reset() {
    u8 v = ram_[0];
    if (!getBitN(v, 4) && !getBitN(v, 5)) {
      set(0xff00, 0xff);
    } else {
      set(0xff00, (v & 0x30 /*get bit 4,5*/) | 0xcf /*reset rest bit*/);
    }
  }

  u8 select_{0xff};
  u8 direction_{0xff};

  MemoryBus* memory_bus_{};
#undef DECL_KEY
};
} // namespace gb
