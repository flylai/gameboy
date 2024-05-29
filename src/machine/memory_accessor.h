#pragma once

#include "common/type.h"
#include "common/utils.h"

namespace gb {

class MemoryAccessor {
public:
  virtual u8 get(u16 addr)           = 0;
  virtual void set(u16 addr, u8 val) = 0;
};

template<u16 N, u16 LO, u16 HI>
class Memory : public MemoryAccessor {
public:
  u8 get(gb::u16 addr) override {
    GB_ASSERT(addr >= LO && addr <= HI);
    return ram[addr];
  }

  void set(gb::u16 addr, gb::u8 val) override {
    GB_ASSERT(addr >= LO && addr <= HI);
    ram[addr] = val;
  }

protected:
  u8 ram[N];
};

static constexpr u16 WORK_RAM_SIZE  = 8 * 1024;
static constexpr u16 VIDEO_RAM_SIZE = 8 * 1024;

class WorkRam : public Memory<WORK_RAM_SIZE, 0xc000, 0xdfff> {};

class VideoRam : public Memory<VIDEO_RAM_SIZE, 0x8000, 0x9fff> {};

} // namespace gb
