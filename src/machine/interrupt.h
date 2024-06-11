#pragma once

#include "common/type.h"
#include "machine/memory/memory_accessor.h"

namespace gb {

enum class InterruptType : u8 {
  kVBLANK = 0,
  kLCD    = 1,
  kTIMER  = 2,
  kSERIAL = 3,
  kJOYPAD = 4,
};

template<u16 LO, u16 HI>
class Interrupt : public Memory<LO, HI> {
public:
  void set(u16 addr, u8 interrupt_type) override {
    Memory<LO, HI>::ram_[0] = clearBitN(Memory<LO, HI>::ram_[0], interrupt_type) | (1 << interrupt_type);
  }

  u8 get(u16 addr) const override { return Memory<LO, HI>::ram_[0]; }
};

class InterruptEnable : public Interrupt<0xffff, 0xffff> {};

class InterruptFlag : public Interrupt<0xff0f, 0xff0f> {};

} // namespace gb
