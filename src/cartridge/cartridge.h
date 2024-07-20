#pragma once

#include <cstring>

#include "cartridge_header.h"
#include "machine/memory/memory_accessor.h"

namespace gb {
class Cartridge : public MemoryAccessor {

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

  virtual ~Cartridge() { free(rom_); }

  const CartridgeHeader& header() const {
    if (rom_) {
      return header_;
    }
    GB_UNREACHABLE();
  }


protected:
  enum class WorkMode : u8 { kSIMPLE, kADVANCE };
  CartridgeHeader header_;

  std::string path_;
  u32 rom_size_{};
  u8* rom_{};
  u8 ram_[0x2000 * 128];

  u8 validRamBankMask() const { return header_.ramSizeByKB() * 1024 / 0x2000 - 1; }

  u8 validRomBankMask() const { return header_.romSizeByKB() * 1024 / 0x4000 - 1; }

  union {
    u8 bank;

    struct {
      u8 rom_bank : 5 {1};
      u8 ram_bank : 2 {0};
      u8 enable_ram : 1 {0};
    };
  } bank_;

  WorkMode work_mode_{WorkMode::kSIMPLE};
};

} // namespace gb
