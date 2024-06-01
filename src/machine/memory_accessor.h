#pragma once

#include "common/type.h"
#include "common/utils.h"

namespace gb {

class MemoryAccessor {
public:
  virtual u8 get(u16 addr)           = 0;
  virtual void set(u16 addr, u8 val) = 0;
};

template<u16 LO, u16 HI>
class Memory : public MemoryAccessor {
public:
  u8 get(gb::u16 addr) override {
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

static constexpr u16 WORK_RAM_SIZE  = 8 * 1024;
static constexpr u16 VIDEO_RAM_SIZE = 8 * 1024;

class WorkRam : public Memory<0xc000, 0xdfff> {};

class VideoRam : public Memory<0x8000, 0x9fff> {};

} // namespace gb
