#pragma once

#include "common/logger.h"
#include "common/type.h"
#include "machine/memory/memory_accessor.h"

namespace gb {
class MemoryBus;

class Timer : public Memory<0xff04, 0xff07> {
public:
  Timer() { reset(); }

  void reset() {
    div_ = 0xac00;
    TIMA(0x00);
    TMA(0x00);
    TAC(0xf8);
  }

  void tick();

  u8 get(u16 addr) const override {
    if (addr == 0xff04) {
      return div_ >> 8;
    } else {
      return ram_[addr - 0xff04];
    }
  }

  void set(u16 addr, u8 val) override {
    GB_ASSERT(addr >= 0xff04 && addr <= 0xff07);
    if (addr == 0xff04) {
      DIV(0);
      ram_[addr - 0xff04] = 0;
    } else {
      ram_[addr - 0xff04] = val;
    }
  }

#define DEF(V)        \
  V(u8, TIMA, 0xff05) \
  V(u8, TMA, 0xff06)  \
  V(u8, TAC, 0xff07)

#define DEF_GET(TYPE, NAME, ADDR) \
  u8 NAME() const { return get(ADDR); }

#define DEF_SET(TYPE, NAME, ADDR) \
  void NAME(u8 val) { set(ADDR, val); }

  DEF(DEF_GET)
  DEF(DEF_SET)

  void DIV(u16);

  bool enableTIMA() const { return getBitN(TAC(), 2); }

  void memoryBus(MemoryBus* memory_bus) { memory_bus_ = memory_bus; }

private:
  u16 div_{};
  u8 tima_reload_counter_{};

  MemoryBus* memory_bus_{};
#undef DEF
#undef DEF_GET
#undef DEF_SET
};

} // namespace gb
