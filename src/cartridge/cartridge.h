#pragma once

#include <cstring>

#include "cartridge_header.h"
#include "machine/memory/memory_accessor.h"

namespace gb {
class Cartridge : public MemoryAccessor {
  enum class WorkMode : u8 { kSIMPLE, kADVANCE };

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
    if (!rom_) {
      GB_LOG(ERROR) << "can not alloc memory " << rom_size_ << " to store the cartridge";
    }

    memset(rom_, 0, rom_size_);
    is.read((char*) rom_, rom_size_);
    is.close();
    header_ = CartridgeHeader(rom_);
  }

  ~Cartridge() { free(rom_); }

  const CartridgeHeader& header() const {
    if (rom_) {
      return header_;
    }
    GB_UNREACHABLE();
  }

  void set(u16 addr, u8 val) override {
    if (addr >= 0x0 && addr <= 0x1fff) {
      if (val & 0xa) {
        bank_ = setBitN(bank_, 7);
      } else {
        bank_ = clearBitN(bank_, 7);
      }
    } else if (addr >= 0x2000 && addr <= 0x3fff) {
      bank_ = (bank_ & 0xe0) /*high 3 bits*/ | (val & 0x1f) /*low 5 bits*/;
    } else if (addr >= 0x4000 && addr <= 0x5fff) {
      bank_ = (bank_ & 0x9f) /* high 2 bits and low 5 bits */ | (val & 0x3 << 5) /*middle 2 bits*/;
    } else if (addr >= 0x6000 && addr <= 0x7fff) {
      if (val & 1) {
        work_mode_ = WorkMode::kADVANCE;
      } else {
        work_mode_ = WorkMode::kSIMPLE;
      }
    }
  }

  u8 get(u16 addr) const override {
    if (addr >= 0x0 && addr <= 0x3fff) {
      return rom_[addr];
    } else if (addr >= 0x4000 && addr <= 0x7fff) {
      return rom_[romBank() * 0x4000 + addr - 0x4000];
    } else if (addr >= 0xa000 && addr <= 0xbfff) {
      if (enableRam()) {
        return ram_[ramBank() * 0x2000 + addr * 0xa000];
      }
      return 0xff;
    }
    GB_UNREACHABLE()
  }

private:
  CartridgeHeader header_;

  std::string path_;
  u32 rom_size_{};
  u8* rom_{};
  u32 ram_size_{};
  u8* ram_{};

  u8 enableRam() const { return getBitN(bank_, 7); }

  u8 romBank() const {
    if (bank_ == 0) {
      return 1;
    }
    if (enableRam()) {
      return bank_ & 0x1f;
    } else {
      return bank_ & 0x7f;
    }
  }

  u8 ramBank() const {
    if (enableRam()) {
      return (bank_ >> 5) & 0x3;
    }
    return 0;
  }

  /*
  u8 enable_ram_:1;
  u8 ram_bank:2;
  u8 rom_bank:5;
  */
  u8 bank_{0};
  WorkMode work_mode_{WorkMode::kSIMPLE};
};

} // namespace gb
