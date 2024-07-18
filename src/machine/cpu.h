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
  void reset() {
    GB_ASSERT(memory_bus_ != nullptr);
    A(0x01);
    F(0xb0);
    B(0x00);
    C(0x13);
    D(0);
    E(0xd8);
    H(0x01);
    L(0x4d);
    pc_ = 0x100;
    sp_ = 0xfffe;
    memory_bus_->set(0xff0f, 0xe1);
    memory_bus_->set(0xffff, 0);
  }

  u8 update(u64 cycle);
  u8 handleInterrupt();

  INLINE u8 A() const { return af_ >> 8; }

  INLINE void A(u8 val) { af_ = (af_ & 0xff) | ((u16) val << 8); }

  INLINE u8 F() const { return af_ & 0xff; }

  INLINE void F(u8 val) { af_ = (af_ & 0xff00) | val; }

  INLINE u16 AF() const { return af_; }

  INLINE void AF(u16 val) {
    af_ = val;
    af_ = af_ & 0xfff0;
  }

  INLINE u8 B() const { return bc_ >> 8; }

  INLINE void B(u8 val) { bc_ = (bc_ & 0xff) | ((u16) val << 8); }

  INLINE u8 C() const { return bc_ & 0xff; }

  INLINE void C(u8 val) { bc_ = (bc_ & 0xff00) | val; }

  INLINE u16 BC() const { return bc_; }

  INLINE void BC(u16 val) {
    tick();
    bc_ = val;
  }

  INLINE u8 D() const { return de_ >> 8; }

  INLINE void D(u8 val) { de_ = (de_ & 0xff) | ((u16) val << 8); }

  INLINE u8 E() const { return de_ & 0xff; }

  INLINE void E(u8 val) { de_ = (de_ & 0xff00) | val; }

  INLINE u16 DE() const { return de_; }

  INLINE void DE(u16 val) {
    tick();
    de_ = val;
  }

  INLINE u8 H() const { return hl_ >> 8; }

  INLINE void H(u8 val) { hl_ = (hl_ & 0xff) | ((u16) val << 8); }

  INLINE u8 L() const { return hl_ & 0xff; }

  INLINE void L(u8 val) { hl_ = (hl_ & 0xff00) | val; }

  INLINE u16 HL() const { return hl_; }

  INLINE void HL(u16 val) {
    tick();
    hl_ = val;
  }

  INLINE u8 zf() const { return getBitN(F(), 7); }

  INLINE void zf(u8 val) { F((val << 7) | clearBitN(F(), 7)); }

  INLINE u8 nf() const { return getBitN(F(), 6); }

  INLINE void nf(u8 val) { F((val << 6) | clearBitN(F(), 6)); }

  INLINE u8 hf() const { return getBitN(F(), 5); }

  INLINE void hf(u8 val) { F((val << 5) | clearBitN(F(), 5)); }

  INLINE u8 cf() const { return getBitN(F(), 4); }

  INLINE void cf(u8 val) { F((val << 4) | clearBitN(F(), 4)); }

  INLINE u16 PC() const { return pc_; }

  INLINE void PC(u16 pc) {
    tick();
    pc_ = pc;
  }

  INLINE void PCWithoutTick(u16 pc) { pc_ = pc; }

  INLINE u16 SP() const { return sp_; }

  INLINE void SP(u16 sp) {
    tick();
    sp_ = sp;
  }

  INLINE bool halt() const { return halt_; }

  INLINE void halt(bool val) { halt_ = val; }

  INLINE bool IME() const { return ime_; }

  INLINE void IME(bool val) {
    if (val) {
      interrupt_delay_ = 1;
    }
    ime_ = val;
  }

  INLINE u8 imm8() {
    auto ret = get(pc_++);
    return ret;
  }

  INLINE u16 imm16() {
    u8 l  = imm8();
    u16 h = imm8();
    return (h << 8) | l;
  }

  INLINE void set(u16 addr, u8 val) {
    tick();
    memory_bus_->set(addr, val);
  }

  INLINE void set16(u16 addr, u16 val) {
    set(addr, val & 0xff);
    set(addr + 1, val >> 8);
  }

  INLINE u8 get(u16 addr) const {
    tick();
    return memory_bus_->get(addr);
  }

  INLINE u8 inc8(u8 val) {
    // https://rgbds.gbdev.io/docs/v0.7.0/gbz80.7#INC_r8
    u8 res = val + 1;
    zf(res == 0);
    nf(0);
    hf((val & 0xf) + 1 > 0xf);
    return res;
  }

  INLINE u8 dec8(u8 val) {
    // https://rgbds.gbdev.io/docs/v0.7.0/gbz80.7#DEC_r8
    u8 res = val - 1;
    zf(res == 0);
    nf(1);
    hf((res & 0xf) == 0xf);
    return res;
  }

  INLINE u8 rlc8(u8 val) {
    u8 res = (val << 1) | (val >> 7);
    zf(res == 0);
    nf(0);
    hf(0);
    cf(val >> 7);
    return res;
  }

  INLINE u16 addHL(u16 val) {
    u16 res = HL() + val;
    nf(0);
    hf((HL() & 0xfff) + (val & 0xfff) > 0xfff);
    cf(HL() > 0xffff - val);
    return res;
  }

  INLINE u8 add8(u8 v1, u8 v2) {
    u16 res = v1 + v2;
    zf((u8) res == 0);
    nf(0);
    hf((v1 & 0xf) + (v2 & 0xf) > 0xf);
    cf(res > 0xff);
    return res;
  }

  INLINE u16 add16(u16 v1, u8 v2) {
    u16 res = v1 + (i8) v2;
    zf(0);
    nf(0);
    hf((v1 & 0xf) + (v2 & 0xf) > 0xf);
    cf((v1 & 0xff) + (v2 & 0xff) > 0xff);
    return res;
  }

  INLINE u8 adc8(u8 v1, u8 v2) {
    u16 res = v1 + v2 + cf();
    zf((u8) res == 0);
    nf(0);
    hf((v1 & 0xf) + (v2 & 0xf) + cf() > 0xf);
    cf(res > 0xff);
    return res;
  }

  INLINE u8 sub8(u8 v1, u8 v2) {
    u8 res = v1 - v2;
    zf(res == 0);
    nf(1);
    hf((v1 & 0xf) < (v2 & 0xf));
    cf(v2 > v1);
    return res;
  }

  INLINE u8 sbc8(u8 v1, u8 v2) {
    u8 res = v1 - v2 - cf();
    zf(res == 0);
    nf(1);
    hf((v1 & 0xf) < (v2 & 0xf) + cf());
    cf(v2 > v1 - cf());
    return res;
  }

  INLINE u8 and8(u8 v1, u8 v2) {
    u8 res = v1 & v2;
    zf(res == 0);
    nf(0);
    hf(1);
    cf(0);
    return res;
  }

  INLINE u8 xor8(u8 v1, u8 v2) {
    u8 res = v1 ^ v2;
    zf(res == 0);
    nf(0);
    hf(0);
    cf(0);
    return res;
  }

  INLINE u8 or8(u8 v1, u8 v2) {
    u8 res = v1 | v2;
    zf(res == 0);
    nf(0);
    hf(0);
    cf(0);
    return res;
  }

  INLINE u8 cp8(u8 v1, u8 v2) {
    u8 res = v1 - v2;
    zf(v1 == v2);
    nf(1);
    hf((v1 & 0xf) < (v2 & 0xf));
    cf(v2 > v1);
    return res;
  }

  INLINE void jr(u8 val) {
    i8 v = val;
    PC(v + PC());
  }

  INLINE void push16(u16 val) {
    set(--sp_, val >> 8);
    set(--sp_, val & 0xff);
  }

  INLINE u16 pop16() {
    u16 res = get(sp_++);
    res     = ((u16) get(sp_++) << 8) | res;
    return res;
  }

  INLINE u8 rrc8(u8 val) {
    u8 res = (val << 7) | (val >> 1);
    zf(res == 0);
    nf(0);
    hf(0);
    cf(val & 0x1);
    return res;
  }

  INLINE u8 rl8(u8 val) {
    u8 res = (val << 1) | cf();
    zf(res == 0);
    nf(0);
    hf(0);
    cf(val >> 7);
    return res;
  }

  INLINE u8 rr8(u8 val) {
    u8 res = (val >> 1) | (cf() << 7);
    zf(res == 0);
    nf(0);
    hf(0);
    cf(val & 0x1);
    return res;
  }

  INLINE u8 sla8(u8 val) {
    u8 res = val << 1;
    zf(res == 0);
    nf(0);
    hf(0);
    cf(val >> 7);
    return res;
  }

  INLINE u8 sra8(u8 val) {
    u8 res = (val >> 1) | (val & 0x80);
    zf(res == 0);
    nf(0);
    hf(0);
    cf(val & 0x1);
    return res;
  }

  INLINE u8 swap8(u8 val) {
    u8 res = (val >> 4) | (val << 4);
    zf(res == 0);
    nf(0);
    hf(0);
    cf(0);
    return res;
  }

  INLINE u8 srl8(u8 val) {
    u8 res = val >> 1;
    zf(res == 0);
    nf(0);
    hf(0);
    cf(val & 0x01);
    return res;
  }

  INLINE u8 bit8(u8 n, u8 val) {
    u8 res = getBitN(val, n);
    zf(res == 0);
    nf(0);
    hf(1);
    return res;
  }

  INLINE u8 res8(u8 n, u8 val) {
    u8 res = clearBitN(val, n);
    return res;
  }

  INLINE u8 set8(u8 n, u8 val) {
    u8 res = setBitN(val, n);
    return res;
  }

  INLINE void tick() const { memory_bus_->tick(); }

  void memoryBus(MemoryBus *memory_bus) { memory_bus_ = memory_bus; }


private:
  u16 af_{}, bc_{}, de_{}, hl_{};
  u16 pc_{}, sp_{};
  bool halt_{};
  bool ime_{};
  u8 interrupt_delay_{};

  MemoryBus *memory_bus_{};
};

} // namespace gb
