#pragma once

#include "cartridge_header.h"
#include "machine/memory/memory_accessor.h"

namespace gb {
class MBC1 : public Cartridge {

public:
  explicit MBC1(const std::string& path) : Cartridge(path) {}

  void set(u16 addr, u8 val) override {
    if (addr <= 0x1fff) {
      // RAM enable
      if ((val & 0xf) == 0xa) {
        bank_.enable_ram = 1;
      } else {
        bank_.enable_ram = 0;
      }
    } else if (addr >= 0x2000 && addr <= 0x3fff) {
      // ROM bank
      val &= 0x1f;
      if (val == 0) {
        val = 1;
      }
      bank_.rom_bank = val & validRomBankMask();
    } else if (addr >= 0x4000 && addr <= 0x5fff) {
      bank_.ram_bank = (val & 0x3);
    } else if (addr >= 0x6000 && addr <= 0x7fff) {
      if (val & 1) {
        work_mode_ = WorkMode::kADVANCE;
      } else {
        work_mode_ = WorkMode::kSIMPLE;
      }
    } else if (addr >= 0xa000 && addr <= 0xbfff) {
      if (!bank_.enable_ram) {
        return;
      }
      if (work_mode_ == WorkMode::kADVANCE) {
        ram_[(bank_.ram_bank & validRamBankMask()) * 0x2000 + addr - 0xa000] = val;
      } else {
        ram_[addr - 0xa000] = val;
      }
    }
  }

  u8 get(u16 addr) const override {
    if (addr <= 0x3fff) {
      if (work_mode_ == WorkMode::kADVANCE) {
        return rom_[((bank_.ram_bank << 5) & validRomBankMask()) * 0x4000 + addr];
      } else {
        return rom_[addr];
      }
    } else if (addr >= 0x4000 && addr <= 0x7fff) {
      return rom_[(((bank_.ram_bank << 5) | bank_.rom_bank) & validRomBankMask()) * 0x4000 + addr - 0x4000];
    } else if (addr >= 0xa000 && addr <= 0xbfff) {
      if (!bank_.enable_ram) {
        return 0xff;
      }
      if (work_mode_ == WorkMode::kADVANCE) {
        return ram_[(bank_.ram_bank & validRamBankMask()) * 0x2000 + addr - 0xa000];
      }
      return ram_[addr - 0xa000];
    }
    GB_UNREACHABLE()
  }
};

} // namespace gb
