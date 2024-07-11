#pragma once

#include "common/logger.h"
#include "common/type.h"
#include "machine/memory/memory_accessor.h"

namespace gb {
class MemoryBus;

class Timer : public Memory<0xff04, 0xff07> {

  struct InternalCounter {
    u8 m_cycle{};
    u8 modulo{};

    u8 update(u8 cycle /* M-cycle */) {
      if (modulo == 0) return 0;
      u16 sum = m_cycle + cycle;
      m_cycle = sum % modulo;
      return sum / modulo;
    }
  };

public:
  Timer() { reset(); }

  void reset() {
    DIV(0x18);
    TIMA(0x00);
    TMA(0x00);
    TAC(0xf8);
    div_counter_.modulo   = 0x40;
    tima_counter_.m_cycle = 0x00;
  }

  u8 update(u64 cycle) {
    // DIV will increase per 4194304/16384=256 T-cycles (64 M-cycles)
    u8 m_cycle = cycle / 4;
    u8 div_n   = div_counter_.update(m_cycle);
    DIV(DIV() + div_n);
    if (enableTIMA()) {
      u8 tima_n = tima_counter_.update(m_cycle);
      for (u8 i = 0; i < tima_n; i++) {
        increaseTIMA();
      }
    }
    return 0;
  }

  void set(u16 addr, u8 val) override {
    GB_ASSERT(addr >= 0xff04 && addr <= 0xff07);
    switch (addr) {
      case 0xff04:
        div_counter_.m_cycle = 0;
        ram_[addr - 0xff04]  = 0;
        break;
      case 0xff05:
      case 0xff06:
        ram_[addr - 0xff04] = val;
        break;
      case 0xff07: {
        if ((TAC() & 0x3) != (val & 0x3)) {
          constexpr u8 tac_clock_table[] = {0xff, 0x4, 0x10, 0x41};
          tima_counter_.m_cycle          = 1;
          tima_counter_.modulo           = tac_clock_table[val & 0x3];
        }
        TIMA(TMA());
        ram_[addr - 0xff04] = val;
        increaseTIMA();
      } break;
      default:
        GB_UNREACHABLE();
    }
  }

#define DEF(V)        \
  V(u8, DIV, 0xff04)  \
  V(u8, TIMA, 0xff05) \
  V(u8, TMA, 0xff06)  \
  V(u8, TAC, 0xff07)

#define DEF_GET(TYPE, NAME, ADDR) \
  u8 NAME() const { return get(ADDR); }

#define DEF_SET(TYPE, NAME, ADDR) \
  void NAME(u8 val) { set(ADDR, val); }

  DEF(DEF_GET)
  DEF(DEF_SET)

  bool enableTIMA() const { return getBitN(TAC(), 2); }

  void memoryBus(MemoryBus *memory_bus) { memory_bus_ = memory_bus; }

private:
  void increaseTIMA();
  MemoryBus *memory_bus_{};

  InternalCounter div_counter_;
  InternalCounter tima_counter_;

#undef DEF
#undef DEF_GET
#undef DEF_SET
};

} // namespace gb
