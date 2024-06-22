#pragma once

#include "common/logger.h"
#include "memory_accessor.h"

namespace gb {
class WorkRam : public Memory<0xc000, 0xdfff> {
public:
  WorkRam() { wram_idx_.set(0xff70, 0); }

  u8 get(gb::u16 addr) const override {
    switch (addr) {
      case 0xc000 ... 0xcfff:
        return Memory::get(addr);
      case 0xd000 ... 0xdfff:
        return ram_[addr - 0xc000 + wram_idx_.get(0xff70) * 0x1000];
      case 0xff70:
        return wram_idx_.get(addr);
      default:
        GB_UNREACHABLE();
    }
    GB_UNREACHABLE();
  }

  void set(gb::u16 addr, gb::u8 val) override {
    switch (addr) {
      case 0xc000 ... 0xcfff:
        Memory::set(addr, val);
        break;
      case 0xd000 ... 0xdfff:
        ram_[addr - 0xc000 + wram_idx_.get(0xff70) * 0x1000] = val;
        break;
      case 0xff70: {
        wram_idx_.set(addr, (val & 0x7) == 0 ? 1 : val);
      } break;
      default:
        GB_UNREACHABLE();
    }
  }

private:
  Memory<0xff70, 0xff70> wram_idx_{};
};


} // namespace gb
