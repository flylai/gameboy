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
  CPU() { reset(); }

  void reset() {
    A(0x01);
    F(0);
    B(0xff);
    C(0x13);
    D(0);
    E(0xc1);
    H(0x84);
    L(0x03);
    PC(0x100);
    SP(0xfffe);
    if_.set(0xff0f, 0xe1);
    ie_.set(0xffff, 0);
  }

  u8 update(u64 cycle);
  u8 handleInterrupt();

  u8 A() const { return af_ >> 8; }

  void A(u8 val) { af_ = (af_ & 0xff) | ((u16) val << 8); }

  u8 F() const { return af_ & 0xff; }

  void F(u8 val) { af_ = (af_ & 0xff00) | val; }

  u16 AF() const { return af_; }

  void AF(u16 val) {
    af_ = val;
    af_ = af_ & 0xfff0;
  }

  u8 B() const { return bc_ >> 8; }

  void B(u8 val) { bc_ = (bc_ & 0xff) | ((u16) val << 8); }

  u8 C() const { return bc_ & 0xff; }

  void C(u8 val) { bc_ = (bc_ & 0xff00) | val; }

  u16 BC() const { return bc_; }

  void BC(u16 val) { bc_ = val; }

  u8 D() const { return de_ >> 8; }

  void D(u8 val) { de_ = (de_ & 0xff) | ((u16) val << 8); }

  u8 E() const { return de_ & 0xff; }

  void E(u8 val) { de_ = (de_ & 0xff00) | val; }

  u16 DE() const { return de_; }

  void DE(u16 val) { de_ = val; }

  u8 H() const { return hl_ >> 8; }

  void H(u8 val) { hl_ = (hl_ & 0xff) | ((u16) val << 8); }

  u8 L() const { return hl_ & 0xff; }

  void L(u8 val) { hl_ = (hl_ & 0xff00) | val; }

  u16 HL() const { return hl_; }

  void HL(u16 val) { hl_ = val; }

  u8 zf() const { return getBitN(F(), 7); }

  void zf(u8 val) { F((val << 7) | clearBitN(F(), 7)); }

  u8 nf() const { return getBitN(F(), 6); }

  void nf(u8 val) { F((val << 6) | clearBitN(F(), 6)); }

  u8 hf() const { return getBitN(F(), 5); }

  void hf(u8 val) { F((val << 5) | clearBitN(F(), 5)); }

  u8 cf() const { return getBitN(F(), 4); }

  void cf(u8 val) { F((val << 4) | clearBitN(F(), 4)); }

  u16 PC() const { return pc_; }

  void PC(u16 pc) { pc_ = pc; }

  u16 SP() const { return sp_; }

  void SP(u16 sp) { sp_ = sp; }

  bool halt() const { return halt_; }

  void halt(bool val) { halt_ = val; }

  bool IME() const { return ime_; }

  void IME(bool val) { ime_ = val; }

  u8 imm8() {
    auto ret = memory_bus_->get(pc_++);
    return ret;
  }

  u16 imm16() {
    u8 l  = memory_bus_->get(pc_++);
    u16 h = memory_bus_->get(pc_++);
    return (h << 8) | l;
  }

  void set(u16 addr, u8 val) { memory_bus_->set(addr, val); }

  u8 get(u16 addr) const { return memory_bus_->get(addr); }

  u8 inc8(u8 val) {
    // https://rgbds.gbdev.io/docs/v0.7.0/gbz80.7#INC_r8
    u8 res = val + 1;
    zf(res == 0);
    nf(0);
    hf((val & 0xf) + 1 > 0xf);
    return res;
  }

  u8 dec8(u8 val) {
    // https://rgbds.gbdev.io/docs/v0.7.0/gbz80.7#DEC_r8
    u8 res = val - 1;
    zf(res == 0);
    nf(1);
    hf((res & 0xf) == 0xf);
    return res;
  }

  u8 rlc8(u8 val) {
    u8 res = (val << 1) | (val & 0x80 >> 7);
    zf(res == 0);
    nf(0);
    hf(0);
    cf(val >> 7);
    return res;
  }

  u16 addHL(u16 val) {
    u16 res = HL() + val;
    nf(0);
    hf((HL() & 0xfff) + (val & 0xfff) > 0xfff);
    cf(HL() > 0xffff - val);
    return res;
  }

  u8 add8(u8 v1, u8 v2) {
    u16 res = v1 + v2;
    zf(res == 0x100);
    nf(0);
    hf((v1 & 0xf) + (v2 & 0xf) > 0xf);
    cf(res > 0xff);
    return res;
  }

  u8 add8(u8 v1, i8 v2) {
    u16 res = v1 + v2;
    zf(res == 0x100);
    nf(0);
    hf((v1 & 0xf + v2 & 0xf) > 0xf);
    cf(res > 0xff);
    return res;
  }

  u8 adc8(u8 v1, u8 v2) {
    u16 res = v1 + v2 + cf();
    zf(res == 0x100);
    nf(0);
    hf((v1 & 0xf) + (v2 & 0xf) + cf() > 0xf);
    cf(res > 0xff);
    return res;
  }

  u8 sub8(u8 v1, u8 v2) {
    u8 res = v1 - v2;
    zf(res == 0);
    nf(1);
    hf((v1 & 0xf) < (v2 & 0xf));
    cf(v2 > v1);
    return res;
  }

  u8 sbc8(u8 v1, u8 v2) {
    u8 res = v1 - v2 - cf();
    zf(res == 0);
    nf(1);
    hf((v1 & 0xf) < (v2 & 0xf) + cf());
    cf(v2 > v1 - cf());
    return res;
  }

  u8 and8(u8 v1, u8 v2) {
    u8 res = v1 & v2;
    zf(res == 0);
    nf(0);
    hf(1);
    cf(0);
    return res;
  }

  u8 xor8(u8 v1, u8 v2) {
    u8 res = v1 ^ v2;
    zf(res == 0);
    nf(0);
    hf(0);
    cf(0);
    return res;
  }

  u8 or8(u8 v1, u8 v2) {
    u8 res = v1 | v2;
    zf(res == 0);
    nf(0);
    hf(0);
    cf(0);
    return res;
  }

  u8 cp8(u8 v1, u8 v2) {
    u8 res = v1 - v2;
    zf(v1 == v2);
    nf(1);
    hf((v1 & 0xf) < (v2 & 0xf));
    cf(v2 > v1);
    return res;
  }

  void jr(u8 val) {
    i8 v = val;
    PC(v + PC());
  }

  void push16(u16 val) {
    set(--sp_, val >> 8);
    set(--sp_, val & 0xff);
  }

  u16 pop16() {
    u16 res = get(sp_++);
    res     = ((u16) get(sp_++) << 8) | res;
    return res;
  }

  u8 rrc8(u8 val) {
    u8 res = (val & 0x1 << 7) | (val >> 1);
    zf(res == 0);
    nf(0);
    hf(0);
    cf(val & 0x1);
    return res;
  }

  u8 rl8(u8 val) {
    u8 res = (val << 1) | cf();
    zf(res == 0);
    nf(0);
    hf(0);
    cf(val >> 7);
    return res;
  }

  u8 rr8(u8 val) {
    u8 res = (val >> 1) | (cf() << 7);
    zf(res == 0);
    nf(0);
    hf(0);
    cf(val & 0x1);
    return res;
  }

  u8 sla8(u8 val) {
    u8 res = val << 1;
    zf(res == 0);
    nf(0);
    hf(0);
    cf(val >> 7);
    return res;
  }

  u8 sra8(u8 val) {
    u8 res = val;
    zf(res == 0);
    nf(0);
    hf(0);
    cf(val & 0x1);
    return res;
  }

  u8 swap8(u8 val) {
    u8 res = (val >> 4) | (val << 4);
    zf(res == 0);
    nf(0);
    hf(0);
    cf(0);
    return res;
  }

  u8 srl8(u8 val) {
    u8 res = val >> 1;
    zf(res == 0);
    nf(0);
    hf(0);
    cf(val & 0x01);
    return res;
  }

  u8 bit8(u8 n, u8 val) {
    u8 res = getBitN(val, n);
    zf(res == 0);
    nf(0);
    hf(1);
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

  void memoryBus(MemoryBus *memory_bus) { memory_bus_ = memory_bus; }

private:
  u16 af_{}, bc_{}, de_{}, hl_{};
  u16 pc_{}, sp_{};
  bool halt_{};
  bool ime_{};

  MemoryBus *memory_bus_{};
  // https://gbdev.io/pandocs/Interrupts.html
  InterruptEnable ie_;
  InterruptFlag if_;
};

} // namespace gb
