#pragma once

#include <cstring>

#include "cartridge_header.h"

namespace gb {
class Cartridge {
public:
  explicit Cartridge(const std::string& path) {
    std::ifstream is(path, std::ios::binary);
    if (!is.is_open()) {
      GB_UNREACHABLE();
    }
    is.seekg(0, std::ios::end);
    rom_size_ = is.tellg();
    is.seekg(0);
    rom_ = static_cast<u8*>(malloc(rom_size_));
    memset(rom_, 0, rom_size_);
    is.read((char*) rom_, rom_size_);
    is.close();
    header_ = CartridgeHeader(rom_);
  }

  const CartridgeHeader& header() const {
    if (rom_) {
      return header_;
    }
    GB_UNREACHABLE();
  }

  void set(u16 addr, u8 val) {
    if (addr >= 0x0 && addr <= 0x1fff) {
      if (val & 0xa) {
        enable_ram_ = true;
      } else {
        enable_ram_ = false;
      }
    } else if (addr >= 0x2000 && addr <= 0x3fff) {
      // todo: check rom_num is valid
      rom_num_ = val & 0x1f;
    } else if (addr >= 0x4000 && addr <= 0x5fff) {

    } else if (addr >= 0x6000 && addr <= 0x7fff) {
      if (val & 1) {
        work_mode_ = kADVANCE;
      } else {
        work_mode_ = kDEFAULT;
      }
    }
  }

  u8 get(u16 addr) {}

private:
  CartridgeHeader header_;

  std::string path_;
  u32 rom_size_{};
  u8* rom_{};
  u32 ram_size_{};
  u8* ram_{};

  bool enable_ram_{false};
  u8 rom_num_{1};
  u8 ram_num_{0};

  enum WorkMode : u8 { kDEFAULT, kADVANCE };

  WorkMode work_mode_{kDEFAULT};
};

} // namespace gb
