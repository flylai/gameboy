#pragma once

#include "common/logger.h"
#include "common/type.h"
#include "interrupt.h"
#include "machine/memory/memory_accessor.h"
#include "machine/memory/memory_bus.h"
#include "machine/ppu/ppu.h"

namespace gb {

class CPU {
public:
  CPU() {}

  void reset() {
    GB_ASSERT(memory_bus_ != nullptr);
    register_.A  = 0x11;
    register_.F  = 0xb0;
    register_.B  = 0x00;
    register_.C  = 0x13;
    register_.D  = 0;
    register_.E  = 0xd8;
    register_.H  = 0x01;
    register_.L  = 0x4d;
    register_.PC = 0x100;
    register_.SP = 0xfffe;
    memory_bus_->set(0xff0f, 0xe1);
    memory_bus_->set(0xffff, 0);
  }

  u8 update(u64 cycle);
  u8 handleInterrupt();

  bool halt() const { return halt_; }

  void halt(bool val) { halt_ = val; }

  bool IME() const { return ime_; }

  void IME(bool val) {
    if (val) {
      interrupt_delay_ = 1;
    }
    ime_ = val;
  }

  u8 imm8() {
    auto ret = memory_bus_->get(register_.PC++);
    return ret;
  }

  u16 imm16() {
    u8 l  = memory_bus_->get(register_.PC++);
    u16 h = memory_bus_->get(register_.PC++);
    return (h << 8) | l;
  }

  void set(u16 addr, u8 val) { memory_bus_->set(addr, val); }

  void set16(u16 addr, u16 val) {
    set(addr, val & 0xff);
    set(addr + 1, val >> 8);
  }

  u8 get(u16 addr) const { return memory_bus_->get(addr); }

  u8 inc8(u8 val) {
    // https://rgbds.gbdev.io/docs/v0.7.0/gbz80.7#INC_r8
    u8 res       = val + 1;
    register_.ZF = res == 0;
    register_.NF = 0;
    register_.HF = (val & 0xf) + 1 > 0xf;
    return res;
  }

  u8 dec8(u8 val) {
    // https://rgbds.gbdev.io/docs/v0.7.0/gbz80.7#DEC_r8
    u8 res       = val - 1;
    register_.ZF = res == 0;
    register_.NF = 1;
    register_.HF = (res & 0xf) == 0xf;
    return res;
  }

  u8 rlc8(u8 val) {
    u8 res       = (val << 1) | (val >> 7);
    register_.ZF = res == 0;
    register_.NF = 0;
    register_.HF = 0;
    register_.CF = val >> 7;
    return res;
  }

  u16 addHL(u16 val) {
    u16 res      = register_.HL + val;
    register_.NF = 0;
    register_.HF = (register_.HL & 0xfff) + (val & 0xfff) > 0xfff;
    register_.CF = register_.HL > 0xffff - val;
    return res;
  }

  u8 add8(u8 v1, u8 v2) {
    u16 res      = v1 + v2;
    register_.ZF = (u8) res == 0;
    register_.NF = 0;
    register_.HF = (v1 & 0xf) + (v2 & 0xf) > 0xf;
    register_.CF = res > 0xff;
    return res;
  }

  u16 add16(u16 v1, u8 v2) {
    u16 res      = v1 + (i8) v2;
    register_.ZF = 0;
    register_.NF = 0;
    register_.HF = (v1 & 0xf) + (v2 & 0xf) > 0xf;
    register_.CF = (v1 & 0xff) + (v2 & 0xff) > 0xff;
    return res;
  }

  u8 adc8(u8 v1, u8 v2) {
    u16 res      = v1 + v2 + register_.CF;
    register_.ZF = (u8) res == 0;
    register_.NF = 0;
    register_.HF = (v1 & 0xf) + (v2 & 0xf) + register_.CF > 0xf;
    register_.CF = res > 0xff;
    return res;
  }

  u8 sub8(u8 v1, u8 v2) {
    u8 res       = v1 - v2;
    register_.ZF = res == 0;
    register_.NF = 1;
    register_.HF = (v1 & 0xf) < (v2 & 0xf);
    register_.CF = v2 > v1;
    return res;
  }

  u8 sbc8(u8 v1, u8 v2) {
    u8 res       = v1 - v2 - register_.CF;
    register_.ZF = res == 0;
    register_.NF = 1;
    register_.HF = (v1 & 0xf) < (v2 & 0xf) + register_.CF;
    register_.CF = v2 > v1 - register_.CF;
    return res;
  }

  u8 and8(u8 v1, u8 v2) {
    u8 res       = v1 & v2;
    register_.ZF = res == 0;
    register_.NF = 0;
    register_.HF = 1;
    register_.CF = 0;
    return res;
  }

  u8 xor8(u8 v1, u8 v2) {
    u8 res       = v1 ^ v2;
    register_.ZF = res == 0;
    register_.NF = 0;
    register_.HF = 0;
    register_.CF = 0;
    return res;
  }

  u8 or8(u8 v1, u8 v2) {
    u8 res       = v1 | v2;
    register_.ZF = res == 0;
    register_.NF = 0;
    register_.HF = 0;
    register_.CF = 0;
    return res;
  }

  u8 cp8(u8 v1, u8 v2) {
    u8 res       = v1 - v2;
    register_.ZF = v1 == v2;
    register_.NF = 1;
    register_.HF = (v1 & 0xf) < (v2 & 0xf);
    register_.CF = v2 > v1;
    return res;
  }

  void jr(u8 val) {
    i8 v         = val;
    register_.PC = v + register_.PC;
  }

  void push16(u16 val) {
    set(--register_.SP, val >> 8);
    set(--register_.SP, val & 0xff);
  }

  u16 pop16() {
    u16 res = get(register_.SP++);
    res     = ((u16) get(register_.SP++) << 8) | res;
    return res;
  }

  u8 rrc8(u8 val) {
    u8 res       = (val << 7) | (val >> 1);
    register_.ZF = res == 0;
    register_.NF = 0;
    register_.HF = 0;
    register_.CF = val & 0x1;
    return res;
  }

  u8 rl8(u8 val) {
    u8 res       = (val << 1) | register_.CF;
    register_.ZF = res == 0;
    register_.NF = 0;
    register_.HF = 0;
    register_.CF = val >> 7;
    return res;
  }

  u8 rr8(u8 val) {
    u8 res       = (val >> 1) | (register_.CF << 7);
    register_.ZF = res == 0;
    register_.NF = 0;
    register_.HF = 0;
    register_.CF = val & 0x1;
    return res;
  }

  u8 sla8(u8 val) {
    u8 res       = val << 1;
    register_.ZF = res == 0;
    register_.NF = 0;
    register_.HF = 0;
    register_.CF = val >> 7;
    return res;
  }

  u8 sra8(u8 val) {
    u8 res       = (val >> 1) | (val & 0x80);
    register_.ZF = res == 0;
    register_.NF = 0;
    register_.HF = 0;
    register_.CF = val & 0x1;
    return res;
  }

  u8 swap8(u8 val) {
    u8 res       = (val >> 4) | (val << 4);
    register_.ZF = res == 0;
    register_.NF = 0;
    register_.HF = 0;
    register_.CF = 0;
    return res;
  }

  u8 srl8(u8 val) {
    u8 res       = val >> 1;
    register_.ZF = res == 0;
    register_.NF = 0;
    register_.HF = 0;
    register_.CF = val & 0x01;
    return res;
  }

  u8 bit8(u8 n, u8 val) {
    u8 res       = getBitN(val, n);
    register_.ZF = res == 0;
    register_.NF = 0;
    register_.HF = 1;
    return res;
  }

  u8 res8(u8 n, u8 val) {
    u8 res = clearBitN(val, n);
    return res;
  }

  u8 set8(u8 n, u8 val) {
    u8 res = setBitN(val, n);
    return res;
  }

  void daa8() {
    u8 a = register_.A;
    u8 v = register_.CF ? 0x60 : 0;
    if (register_.HF) {
      v |= 0x06;
    }
    if (!register_.NF) {
      if ((a & 0x0f) > 0x09) {
        v |= 0x06;
      }
      if (a > 0x99) {
        v |= 0x60;
      }
      a += v;
    } else {
      a -= v;
    }
    register_.CF = v >= 0x60;
    register_.HF = 0;
    register_.ZF = a == 0x0;
    register_.A  = a;
  }

  void cpl8() {
    register_.NF = 1;
    register_.HF = 1;
    register_.A  = ~register_.A;
  }

  void scf8() {
    register_.NF = 0;
    register_.HF = 0;
    register_.CF = 1;
  }

  void ccf8() {
    register_.NF = 0;
    register_.HF = 0;
    register_.CF = !register_.CF;
  }

  void memoryBus(MemoryBus *memory_bus) { memory_bus_ = memory_bus; }

private:
  //  u16 af_{}, bc_{}, de_{}, hl_{};
  //  u16 register_.PC{}, register_.SP{};
  bool halt_{};
  bool ime_{};
  u8 interrupt_delay_{};

  void tick(u8) {}

public:
  struct {
    union {
      struct {
        union {
          struct {
            u8 dummy : 4;
            u8 CF : 1;
            u8 HF : 1;
            u8 NF : 1;
            u8 ZF : 1;
          };

          u8 F;
        };

        u8 A;
      };

      u16 AF{};
    };

    union {
      struct {
        u8 C;
        u8 B;
      };

      u16 BC{};
    };

    union {
      struct {
        u8 E;
        u8 D;
      };

      u16 DE{};
    };

    union {
      struct {
        u8 L;
        u8 H;
      };

      u16 HL{};
    };

    u16 PC{};
    u16 SP{};
  } register_;

  MemoryBus *memory_bus_{};
};

} // namespace gb
