#pragma once

#include "common/logger.h"
#include "memory_accessor.h"

namespace gb {
class WorkRam : public MemoryAccessor {
public:
  WorkRam() { wram_idx_.set(0xff70, 0); }

  u8 get(gb::u16 addr) const override {
    switch (addr) {
      case 0xc000 ... 0xcfff:
        return ram_[addr - 0xc000];
      case 0xd000 ... 0xdfff:
        return ram_[addr - 0xc000 + wram_idx_.get(0xff70) * 0x1000];
      case 0xe000 ... 0xfdff:
        return get(addr - 0xe000 + 0xc000);
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
        ram_[addr - 0xc000] = val;
        break;
      case 0xd000 ... 0xdfff:
        ram_[(u16) (addr - 0xc000 + wram_idx_.get(0xff70) * 0x1000)] = val;
        break;
      case 0xe000 ... 0xfdff:
        return set(addr - 0xe000 + 0xc000, val);
      case 0xff70:
        val = val & 0x7;
        wram_idx_.set(addr, val == 0 ? 1 : val);
        break;
      default:
        GB_UNREACHABLE();
    }
  }

private:
  u8 ram_[0x8000]{};
  Memory<0xff70, 0xff70> wram_idx_{};
};


} // namespace gb
