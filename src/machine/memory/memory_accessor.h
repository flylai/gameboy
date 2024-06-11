#pragma once

#include "common/type.h"
#include "common/utils.h"

namespace gb {

class MemoryAccessor {
public:
  virtual u8 get(u16 addr) const     = 0;
  virtual void set(u16 addr, u8 val) = 0;
};

template<u32 LO, u32 HI>
class Memory : public MemoryAccessor {
public:
  u8 get(gb::u16 addr) const override {
    GB_ASSERT(addr >= LO && addr <= HI);
    return ram_[addr - LO];
  }

  void set(gb::u16 addr, gb::u8 val) override {
    GB_ASSERT(addr >= LO && addr <= HI);
    ram_[addr - LO] = val;
  }

protected:
  u8 ram_[HI - LO + 1]{};
};

} // namespace gb
