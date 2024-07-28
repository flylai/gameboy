#pragma once

#include "common/type.h"
#include "machine/memory/memory_accessor.h"

namespace gb {

enum InterruptType : u8 {
  kVBLANK = 0,
  kLCDSTAT = 1,
  kTIMER  = 2,
  kSERIAL = 3,
  kJOYPAD = 4,
};

template<u16 LO, u16 HI>
class Interrupt : public Memory<LO, HI> {
public:
  void irq(InterruptType interrupt_type) {
    u8 val = Memory<LO, HI>::get(LO);
    val    = clearBitN(val, interrupt_type) | (1 << interrupt_type);
    Memory<LO, HI>::set(LO, val);
  }
};

class InterruptEnable : public Interrupt<0xffff, 0xffff> {};

class InterruptFlag : public Interrupt<0xff0f, 0xff0f> {};

} // namespace gb
