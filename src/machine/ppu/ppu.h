#pragma once

#include <array>
#include <queue>

#include "common/logger.h"
#include "common/type.h"
#include "machine/memory/memory_accessor.h"
#include "object_attribute.h"
#include "ppu_register.h"

namespace gb {
class MemoryBus;

class LCDData : public Memory<0, 160 * 144 * 4> {
public:
  u8 *get() { return ram_; }
};

class PPU : public MemoryAccessor {
public:
  void tick() {}

  u8 get(u16 addr) const override {
    return 0xff;
    return ppu_reg_.get(addr);
  }

  void set(u16 addr, u8 val) override { ppu_reg_.set(addr, val); }

  void memoryBus(MemoryBus *memory_bus) {
    memory_bus_ = memory_bus;
    ppu_reg_.memoryBus(memory_bus);
  }

  LCDData *lcdData() { return &lcd_data_; }

private:
  MemoryBus *memory_bus_;
  PPURegister ppu_reg_;
  LCDData lcd_data_;
};

} // namespace gb
