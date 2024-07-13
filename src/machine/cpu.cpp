#include "cpu.h"

#include <memory>
#include <string>

#include "common/type.h"

namespace gb {

// https://gbdev.io/gb-opcodes//optables/
// https://rgbds.gbdev.io/docs/v0.7.0/gbz80.7
// https://gb-archive.github.io/salvage/decoding_gbz80_opcodes/Decoding%20Gamboy%20Z80%20Opcodes.html

static constexpr u16 OFFSET = 0xff00;

#define A cpu->register_.A
#define F cpu->register_.F
#define AF cpu->register_.AF
#define B cpu->register_.B
#define C cpu->register_.C
#define BC cpu->register_.BC
#define D cpu->register_.D
#define E cpu->register_.E
#define DE cpu->register_.DE
#define H cpu->register_.H
#define L cpu->register_.L
#define HL cpu->register_.HL
#define ZF cpu->register_.ZF
#define NF cpu->register_.NF
#define HF cpu->register_.HF
#define CF cpu->register_.CF
#define PC cpu->register_.PC
#define SP cpu->register_.SP
#define halt(...) cpu->halt(__VA_ARGS__)
#define imm8(...) cpu->imm8(__VA_ARGS__)
#define imm16(...) cpu->imm16(__VA_ARGS__)
#define set(...) cpu->set(__VA_ARGS__)
#define set16(...) cpu->set16(__VA_ARGS__)
#define get(...) cpu->get(__VA_ARGS__)
#define inc8(...) cpu->inc8(__VA_ARGS__)
#define dec8(...) cpu->dec8(__VA_ARGS__)
#define rlc8(...) cpu->rlc8(__VA_ARGS__)
#define addHL(...) cpu->addHL(__VA_ARGS__)
#define add8(...) cpu->add8(__VA_ARGS__)
#define add16(...) cpu->add16(__VA_ARGS__)
#define adc8(...) cpu->adc8(__VA_ARGS__)
#define sub8(...) cpu->sub8(__VA_ARGS__)
#define sub16(...) cpu->sub16(__VA_ARGS__)
#define sbc8(...) cpu->sbc8(__VA_ARGS__)
#define and8(...) cpu->and8(__VA_ARGS__)
#define xor8(...) cpu->xor8(__VA_ARGS__)
#define or8(...) cpu->or8(__VA_ARGS__)
#define cp8(...) cpu->cp8(__VA_ARGS__)
#define jr(...) cpu->jr(__VA_ARGS__)
#define push16(...) cpu->push16(__VA_ARGS__)
#define pop16(...) cpu->pop16(__VA_ARGS__)
#define IME(...) cpu->IME(__VA_ARGS__)
#define rrc8(...) cpu->rrc8(__VA_ARGS__)
#define rl8(...) cpu->rl8(__VA_ARGS__)
#define rr8(...) cpu->rr8(__VA_ARGS__)
#define sla8(...) cpu->sla8(__VA_ARGS__)
#define sra8(...) cpu->sra8(__VA_ARGS__)
#define swap8(...) cpu->swap8(__VA_ARGS__)
#define srl8(...) cpu->srl8(__VA_ARGS__)
#define bit8(...) cpu->bit8(__VA_ARGS__)
#define res8(...) cpu->res8(__VA_ARGS__)
#define set8(...) cpu->set8(__VA_ARGS__)
#define daa8(...) cpu->daa8()
#define cpl8(...) cpu->cpl8()
#define scf8(...) cpu->scf8()
#define ccf8(...) cpu->ccf8()

/*
let unprefixed = json["unprefixed"]
for (let op in unprefixed) {
  let obj = unprefixed[op];
  let op_name = obj["mnemonic"];
  let str = op_name;
  for (let operand in obj['operands']) {
    str += "_"
    if (!obj['operands'][operand]['immediate']) {
      str += "x" + obj['operands'][operand]['name'] + "x";
    } else {
      str += obj['operands'][operand]['name'];
    }
  }
  let flags = obj["flags"];
  str += "_" + (flags['Z'] == '-' ? "x" : flags['Z']);
  str += "_" + (flags['N'] == '-' ? "x" : flags['N']);
  str += "_" + (flags['H'] == '-' ? "x" : flags['H']);
  str += "_" + (flags['C'] == '-' ? "x" : flags['C']);
  str += "," + op + "," + obj["bytes"] + "," + obj["cycles"].at(-1);
  console.log(str)
}
*/

u8 r8(CPU *cpu, u8 i) {
  GB_ASSERT(i < 8);
  if (i != 6) [[likely]] {
    static u8 *r[] = {&B, &C, &D, &E, &H, &L, nullptr, &A};
    return *r[i];
  }
  return get(HL);
}

void r8(CPU *cpu, u8 i, u8 v) {
  GB_ASSERT(i < 8);
  if (i != 6) [[likely]] {
    static u8 *r[] = {&B, &C, &D, &E, &H, &L, nullptr, &A};
    *r[i]          = v;
  } else {
    set(HL, v);
  }
}

u16 &r16(CPU *cpu, u8 i) {
  GB_ASSERT(i < 4);
  static u16 *r[] = {&BC, &DE, &HL, &SP};
  return *r[i];
}

u16 &r16stk(CPU *cpu, u8 i) {
  GB_ASSERT(i < 4);
  static u16 *r[] = {&BC, &DE, &HL, &AF};
  return *r[i];
}

u16 &r16mem(CPU *cpu, u8 i) {
  GB_ASSERT(i < 4);
  static u16 *r[] = {&BC, &DE, &HL, &HL};
  return *r[i];
}

bool cond(CPU *cpu, u8 i) {
  GB_ASSERT(i < 4);
  if (i == 0) {
    return !ZF;
  } else if (i == 1) {
    return ZF;
  } else if (i == 2) {
    return !CF;
  } else if (i == 3) {
    return CF;
  }
  GB_UNREACHABLE();
}

auto alu(CPU *cpu, u8 i) {
  static const std::function<u8(u8, u8)> f[] = {
          [cpu](u8 a, u8 b) { return add8(a, b); },
          [cpu](u8 a, u8 b) { return adc8(a, b); },
          [cpu](u8 a, u8 b) { return sub8(a, b); },
          [cpu](u8 a, u8 b) { return sbc8(a, b); },
          [cpu](u8 a, u8 b) { return and8(a, b); },
          [cpu](u8 a, u8 b) { return xor8(a, b); },
          [cpu](u8 a, u8 b) { return or8(a, b); },
          [cpu](u8 a, u8 b) {
            u8 ret = a;
            cp8(a, b);
            return ret;
          },
  };
  return f[i];
}

void block0(CPU *cpu, u8 opcode) {
  GB_ASSERT((opcode >> 6) == 0); // high 7, 6 bit are both 0.
  u8 y = (opcode & 0x3f) >> 3;   // high 5, 4, 3 bit
  u8 z = opcode & 0x7;           // low 2, 1, 0 bit
  u8 p = y >> 1;                 // high 5, 4 bit
  u8 q = y & 0x1;                // bit 3

  if (z == 0) {
    switch (y) {
      case 0: {
        // NOP
        break;
      }
      case 1: {
        // LD imm16, SP
        set16(imm16(), SP);
        break;
      }
      case 2: {
        // STOP
        break;
      }
      case 3: {
        jr(imm8());
        break;
      }
      case 4 ... 7: {
        bool cc   = cond(cpu, y - 4);
        u8 target = imm8();
        if (cc) {
          jr(target);
        }
        break;
      }
      default:
        GB_UNREACHABLE();
    }
    return;
  }

  if (z == 1) {
    if (q == 0) {
      // LD r16, imm16
      r16(cpu, p) = imm16();
    } else if (q == 1) {
      // ADD HL, r16
      HL = addHL(r16(cpu, p));
    } else {
      GB_UNREACHABLE();
    }
    return;
  }

  if (z == 2) {
    if (q == 0) {
      set(r16mem(cpu, p), A);
    }
    if (q == 1) {
      A = get(r16mem(cpu, p));
    }

    if (p == 2) {
      HL++;
    } else if (p == 3) {
      HL--;
    }
    return;
  }

  if (z == 3) {
    if (q == 0) {
      // INC r16
      r16(cpu, p)++;
    }
    if (q == 1) {
      // DEC r16
      r16(cpu, p)--;
    }
    return;
  }

  if (z == 4) {
    // INC r8
    r8(cpu, y, inc8(r8(cpu, y)));
    return;
  }

  if (z == 5) {
    // DEC r8
    r8(cpu, y, dec8(r8(cpu, y)));
    return;
  }

  if (z == 6) {
    // LD r8, imm8
    r8(cpu, y, imm8());
    return;
  }

  if (z == 7) {
    const static std::function<void()> f[] = {
            [cpu]() {
              // RLCA
              A  = rlc8(A);
              ZF = 0;
            },
            [cpu]() {
              // RRCA
              ZF = 0;
              NF = 0;
              HF = 0;
              CF = A & 0x1;
              A  = (A >> 1 | ((A & 0x7f) << 7));
            },
            [cpu]() {
              // RLA
              ZF   = 0;
              NF   = 0;
              HF   = 0;
              u8 c = CF;
              CF   = (A >> 7);
              A    = (A << 1 | c);
            },
            [cpu]() {
              // RRA
              A  = rr8(A);
              ZF = 0;
            },
            [cpu]() { daa8(); },
            [cpu]() { cpl8(); },
            [cpu]() { scf8(); },
            [cpu]() { ccf8(); },
    };
    f[y]();
    return;
  }
}

void block1(CPU *cpu, u8 opcode) {
  GB_ASSERT((opcode >> 6) == 1);
  u8 y = (opcode & 0x3f) >> 3; // high 5, 4, 3 bit
  u8 z = opcode & 0x7;         // low 2, 1, 0 bit
  u8 p = y >> 1;               // high 5, 4 bit
  u8 q = y & 0x1;              // bit 3
  if (z == 6 && y == 6) {
    halt();
    return;
  }
  r8(cpu, y, r8(cpu, z));
}

void block2(CPU *cpu, u8 opcode) {
  GB_ASSERT((opcode >> 6) == 2);
  u8 y                  = (opcode & 0x3f) >> 3; // high 5, 4, 3 bit
  u8 z                  = opcode & 0x7;         // low 2, 1, 0 bit
  [[maybe_unused]] u8 p = y >> 1;               // high 5, 4 bit
  [[maybe_unused]] u8 q = y & 0x1;              // bit 3
  A                     = alu(cpu, y)(A, r8(cpu, z));
}

void block3(CPU *cpu, u8 opcode) {
  GB_ASSERT((opcode >> 6) == 3);
  u8 y = (opcode & 0x3f) >> 3; // high 5, 4, 3 bit
  u8 z = opcode & 0x7;         // low 2, 1, 0 bit
  u8 p = y >> 1;               // high 5, 4 bit
  u8 q = y & 0x1;              // bit 3

  if (z == 0) {
    switch (y) {
      case 0 ... 3: {
        if (cond(cpu, y)) {
          PC = pop16();
        }
      } break;
      case 4: {
        set(OFFSET + imm8(), A);
      } break;
      case 5: {
        SP = add16(SP, imm8());
      } break;
      case 6: {
        A = get(OFFSET + imm8());
      } break;
      case 7: {
        HL = add16(SP, imm8());
      } break;
      default:
        GB_UNREACHABLE();
    }
    return;
  }

  if (z == 1) {
    if (q == 0) {
      r16stk(cpu, p) = pop16();
      // workaround
      if (p == 3) [[unlikely]] {
        cpu->register_.dummy = 0;
      }
      return;
    }
    // q == 1
    switch (p) {
      case 0: {
        PC = pop16();
        break;
      }
      case 1: {
        IME(1);
        PC = pop16();
        break;
      }
      case 2: {
        // JP HL
        PC = HL;
        break;
      }
      case 3: {
        // LD SP, HL
        SP = HL;
        break;
      }
      default:
        GB_UNREACHABLE();
    }
    return;
  }

  if (z == 2) {
    switch (y) {
      case 0 ... 3: {
        u16 target = imm16();
        if (cond(cpu, y)) {
          PC = target;
        }
      } break;
      case 4: {
        // LD [0xff00+C], A
        set(OFFSET + C, A);
      } break;
      case 5: {
        // LD [a16], A
        set(imm16(), A);
      } break;
      case 6: {
        // LD A, [0xff00+C]
        A = get(OFFSET + C);
      } break;
      case 7: {
        // LD A, [a16]
        A = get(imm16());
      } break;
      default:
        GB_UNREACHABLE();
    }
    return;
  }

  if (z == 3) {
    if (y == 0) {
      PC = imm16();
    } else if (y == 1) {
      u8 cb   = imm8();
      u8 cb_z = cb & 0x7;
      u8 cb_y = (cb >> 3) & 0x7;
      u8 cb_x = (cb >> 6) & 0x3;
      switch (cb_x) {
        case 0: {
          static const std::function<u8(u8)> f[] = {
                  [cpu](u8 a) { return rlc8(a); },  [cpu](u8 a) { return rrc8(a); },
                  [cpu](u8 a) { return rl8(a); },   [cpu](u8 a) { return rr8(a); },
                  [cpu](u8 a) { return sla8(a); },  [cpu](u8 a) { return sra8(a); },
                  [cpu](u8 a) { return swap8(a); }, [cpu](u8 a) { return srl8(a); },
          };
          r8(cpu, cb_z, f[cb_y](r8(cpu, cb_z)));
        } break;
        case 1: {
          bit8(cb_y, r8(cpu, cb_z));
        } break;
        case 2: {
          r8(cpu, cb_z, res8(cb_y, r8(cpu, cb_z)));
        } break;
        case 3: {
          r8(cpu, cb_z, set8(cb_y, r8(cpu, cb_z)));
        } break;
        default:
          GB_UNREACHABLE();
      }
    } else if (y == 6) {
      IME(0);
    } else if (y == 7) {
      IME(1);
    } else {
      GB_UNREACHABLE();
    }
    return;
  }

  if (z == 4) {
    if (y >= 0 && y <= 3) {
      // call ...
      u16 target = imm16();
      if (cond(cpu, y)) {
        push16(PC);
        PC = target;
      }
    } else {
      GB_UNREACHABLE();
    }
    return;
  }

  if (z == 5) {
    if (q == 0) {
      // PUSH
      push16(r16stk(cpu, p));
    } else {
      // q == 1
      if (p == 0) {
        // CALL
        u16 target = imm16();
        push16(PC);
        PC = target;
      } else {
        GB_UNREACHABLE();
      }
    }
  }

  if (z == 6) {
    A = alu(cpu, y)(A, imm8());
    return;
  }

  if (z == 7) {
    push16(PC);
    PC = y * 8;
  }
}

#undef A
#undef F
#undef AF
#undef B
#undef C
#undef BC
#undef D
#undef E
#undef DE
#undef H
#undef L
#undef HL
#undef ZF
#undef NF
#undef HF
#undef CF
#undef PC
#undef SP
#undef halt
#undef imm8
#undef imm16
#undef set
#undef set16
#undef get
#undef inc8
#undef dec8
#undef rlc8
#undef addHL
#undef add8
#undef add16
#undef adc8
#undef sub8
#undef sub16
#undef sbc8
#undef and8
#undef xor8
#undef or8
#undef cp8
#undef jr
#undef push16
#undef pop16
#undef IME
#undef rrc8
#undef rl8
#undef rr8
#undef sla8
#undef sra8
#undef swap8
#undef srl8
#undef bit8
#undef res8
#undef set8
#undef daa8
#undef cpl8
#undef scf8
#undef ccf8

u8 CPU::update(u64 cycle) {
  if (halt()) {
    // https://gbdev.io/pandocs/halt.html#halt-bug
    // todo: if IME() not set but ie/if is set, we need to handle this bug.
    if (IME() || (memory_bus_->get(0xff0f) & memory_bus_->get(0xffff))) {
      halt(false);
    } else {
      // idle
    }
    return 4;
  } else {
    // handle interrupt first
    u8 irq = handleInterrupt();
    if (irq != 0) {
      return irq;
    }
    u8 inst_idx = imm8();

    GB_ASSERT(inst_idx <= 255 || inst_idx >= 0);
    u8 x = inst_idx >> 6;
    if (x == 0) block0(this, inst_idx);
    if (x == 1) block1(this, inst_idx);
    if (x == 2) block2(this, inst_idx);
    if (x == 3) block3(this, inst_idx);

    return 0;
    //    return instruction_table()[inst_idx](this);
  }
  GB_UNREACHABLE();
}

u8 CPU::handleInterrupt() {
  if (!IME()) {
    return 0;
  }
  if (interrupt_delay_ != 0) {
    interrupt_delay_--;
    return 0;
  }
  u8 interrupt_mask = memory_bus_->get(0xffff) & memory_bus_->get(0xff0f);
  if (interrupt_mask == 0) {
    return 0;
  }
  constexpr u16 interrupt_table[] = {
          0x40, // VBLANK
          0x48, // STAT
          0x50, // TIMER
          0x58, // SERIAL
          0x60, // JOYPAD
  };
  // interrupt priority
  // https://gbdev.io/pandocs/Interrupts.html#interrupt-priorities
  for (u8 i = 0; i < sizeof(interrupt_table) / sizeof(interrupt_table[0]); i++) {
    if (interrupt_mask & (1 << i)) {
      push16(register_.PC);
      register_.PC = (interrupt_table[i]);
      memory_bus_->set(0xff0f, memory_bus_->get(0xff0f) & ~(1 << i));
      break;
    }
  }
  IME(false);
  // irq wasted 5 M-cycles = 20 i-cycles
  return 20;
}


} // namespace gb
