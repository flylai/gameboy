#pragma once
#include "machine/memory/memory_accessor.h"

namespace gb {
class MemoryBus;

class Joypad : public Memory<0xff00, 0xff00> {
public:
  Joypad() { ram_[0] = 0xff; }

  u8 update(u64) { return 0; }

#define DEF_KEY(NAME, CHECK_BIT, REG_BIT) \
  void NAME(bool press) {                 \
    if (!getBitN(ram_[0], CHECK_BIT)) {   \
      return;                             \
    }                                     \
    if (press) {                          \
      p1_##REG_BIT##_clear();             \
    } else {                              \
      p1_##REG_BIT##_set();               \
    }                                     \
  }

  DEF_KEY(A, 5, 0);
  DEF_KEY(B, 5, 1);
  DEF_KEY(Select, 5, 2);
  DEF_KEY(Start, 5, 3);

  DEF_KEY(Right, 4, 0)
  DEF_KEY(Left, 4, 1)
  DEF_KEY(Up, 4, 2)
  DEF_KEY(Down, 4, 3)

  void set(gb::u16 addr, gb::u8 val) override {
    u8 p1   = ram_[0];
    val     = (val & 0x30) | (p1 & 0xcf);
    ram_[0] = val;
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

  // A / Right
  void p1_0_set();
  void p1_0_clear();

  // B / Left
  void p1_1_set();
  void p1_1_clear();

  // Select / Up
  void p1_2_set();
  void p1_2_clear();

  // Start / Down
  void p1_3_set();
  void p1_3_clear();

  MemoryBus* memory_bus_{};
#undef DEF_KEY
};
} // namespace gb
