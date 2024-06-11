#pragma once

#include "common/logger.h"
#include "common/type.h"
#include "machine/memory/memory_accessor.h"

namespace gb {
class Timer : public Memory<0xff04, 0xff07> {
public:
  Timer() { reset(); }

  void reset() {
    _DIV(0x18);
    _TIMA(0x00);
    TMA(0);
    TAC(0xf8);
  }

  u8 update(u64 cycle) {
    constexpr u8 tac_clock_table[] = {0xff, 0x4, 0xf, 0x40};
    u8 selected_clock              = TAC() & 0x3;
    if (cycle % tac_clock_table[selected_clock] == 0) {
      increaseTIMA();
    }
    // DIV is incremented at a rate of 16384 Hz
    // we know that RTC uses 4194304 Hz
    // so DIV will increase every 419304/16384=25.59(approx.)
    if ((cycle % 25) == 0) {
      increaseDIV();
    }
    return 0;
  }

  void set(u16 addr, u8 val) override {
    GB_ASSERT(addr >= 0xff04 && addr <= 0xff07);
    switch (addr) {
      case 0xff04:
        Memory::set(addr, 0);
        break;
      case 0xff05:
        Memory::set(addr, val);
        break;
      case 0xff06:
        Memory::set(addr, val);
        break;
      case 0xff07:
        Memory::set(addr, (TAC() & 0xf8) | (val & 0x7));
        break;
      default:
        GB_UNREACHABLE();
    }
  }

#define DEF(V)         \
  V(u8, _DIV, 0xff04)  \
  V(u8, _TIMA, 0xff05) \
  V(u8, TMA, 0xff06)   \
  V(u8, TAC, 0xff07)

#define DEF_GET(TYPE, NAME, ADDR) \
  u8 NAME() const { return get(ADDR); }

#define DEF_SET(TYPE, NAME, ADDR) \
  void NAME(u8 val) { set(ADDR, val); }

  DEF(DEF_GET)
  DEF(DEF_SET)

  bool enable() const { return getBitN(TAC(), 2); }

  void DIV(u8) {
    // todo: when CPU execute STOP, DIV will reset too.
    _DIV(0);
  }

  u8 DIV() const { return _DIV(); }

private:
  void increaseDIV() { _DIV(_DIV() + 1); }

  void increaseTIMA() {
    if (!enable()) {
      return;
    }
    _TIMA(_TIMA() + 1);
    if (_TIMA() == 0) {
      // overflow
      _TIMA(TMA());
      // todo: irq
    }
  }

#undef DEF
#undef DEF_GET
#undef DEF_SET
};

} // namespace gb
