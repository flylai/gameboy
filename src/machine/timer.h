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
    _DIV(0x18);
    _TIMA(0x00);
    TMA(0x00);
    TAC(0xf8);
  }

  u8 update(u64 cycle) {
    constexpr u8 tac_clock_table[] = {0xff, 0x4, 0xf, 0x40};
    u8 selected_clock              = TAC() & 0x3;
    GB_ASSERT(selected_clock <= sizeof(tac_clock_table) / sizeof(tac_clock_table[0]));

    if (cycle >= tima_next_cycle_) {
      increaseTIMA();
      tima_next_cycle_ += tac_clock_table[selected_clock];
    }

    // DIV is incremented at a rate of 16384 Hz
    // we know that RTC uses 4194304 Hz
    // so DIV will increase every 419304/16384=25.59(approx.)
    if (cycle >= div_next_cycle_) {
      increaseDIV();
      div_next_cycle_ += 25;
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
        Memory::set(addr, val);
        break;
      default:
        Memory::set(addr, val);
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
    // todo: when CPU execute `STOP`, DIV will reset too.
    _DIV(0);
  }

  u8 DIV() const { return _DIV(); }

  void memoryBus(MemoryBus *memory_bus) { memory_bus_ = memory_bus; }

private:
  void increaseDIV() { _DIV(_DIV() + 1); }

  void increaseTIMA();

  u64 tima_next_cycle_{};
  u64 div_next_cycle_{};
  MemoryBus *memory_bus_{};

#undef DEF
#undef DEF_GET
#undef DEF_SET
};

} // namespace gb
