#pragma once

#include <array>
#include <fstream>

#include "common/logger.h"
#include "common/type.h"
#include "common/utils.h"

namespace gb {

// https://gbdev.io/pandocs/The_Cartridge_Header.html

class CartridgeHeader {
public:
  static constexpr int HEADER_LEN = 0x150 - 0x100;
  CartridgeHeader()               = default;

  CartridgeHeader(u8 *rom) : rom_{rom} {}

  enum Type : u8 {
    kROM_ONLY                       = 0x00,
    kMBC1                           = 0x01,
    kMBC1_RAM                       = 0x02,
    kMBC1_RAM_BATTERY               = 0x03,
    kMBC2                           = 0x05,
    kMBC2_BATTERY                   = 0x06,
    kROM_RAM                        = 0x08,
    kROM_RAM_BATTERY                = 0x09,
    kMMM01                          = 0x0B,
    kMMM01_RAM                      = 0x0C,
    kMMM01_RAM_BATTERY              = 0x0D,
    kMBC3_TIMER_BATTERY             = 0x0F,
    kMBC3_TIMER_RAM_BATTERY         = 0x10,
    kMBC3                           = 0x11,
    kMBC3_RAM                       = 0x12,
    kMBC3_RAM_BATTERY               = 0x13,
    kMBC5                           = 0x19,
    kMBC5_RAM                       = 0x1A,
    kMBC5_RAM_BATTERY               = 0x1B,
    kMBC5_RUMBLE                    = 0x1C,
    kMBC5_RUMBLE_RAM                = 0x1D,
    kMBC5_RUMBLE_RAM_BATTERY        = 0x1E,
    kMBC6                           = 0x20,
    kMBC7_SENSOR_RUMBLE_RAM_BATTERY = 0x22,
    kPOCKET_CAMERA                  = 0xFC,
    kBANDAI_TAMA5                   = 0xFD,
    kHuC3                           = 0xFE,
    kHuC1_RAM_BATTERY               = 0xFF,
  };

  u32 entry() const {
    return (rom_[0x100] << 24) & //
           (rom_[0x101] << 16) & //
           (rom_[0x102] << 8) &  //
           (rom_[0x103]);
  }

  bool checkHeader /*Nintendo logo*/ () const {
    // 0x0104-0x0133
    for (u16 i = 0x104; i < 0x133; i++) {
      if (rom_[i] != nintendo_logo_[i - 0x104]) {
        return false;
      }
    }
    return true;
  }

  std::string_view title() const {
    // 0x0134-0x013E
    u16 end_addr = 0x13e;
    for (u16 i = 0x0134; i <= end_addr; i++) {
      if (rom_[i] == 0) {
        end_addr = i;
        break;
      }
    }
    char buf[222];
    sprintf(buf, "%s", rom_[0x134]);
    return {};
    return {reinterpret_cast<char *>(rom_[0x134]), end_addr - 0x134u};
  }

  u32 manufacturerCode() const {
    // 0x013F-0x0142
    return (rom_[0x13f] << 24) & //
           (rom_[0x140] << 16) & //
           (rom_[0x141] << 8) &  //
           (rom_[0x142]);
  }

  u8 cgbFlag() const {
    // 0x0143
    return rom_[0x143];
  }

  u16 newLicenseCode() const {
    // 0x144-0x0145
    if (oldLicenseCode() != 0x33) {
      // no sense
      return 0x0;
    }
    return (rom_[0x144] << 8) & rom_[0x145];
  }

  u8 sgbFlag() const {
    // 0x0146
    return rom_[0x146];
  }

  Type type() const {
    // 0x0147
    return static_cast<Type>(rom_[0x147]);
  }

  u8 romSize() const {
    // 0x0148
    // 32KiB * (1 << rom_[0x148])
    return rom_[0x148];
  }

  u16 romSizeByKB() const {
    if (romSize() <= 8) {
      return 32 * (1 << romSize());
    } else {
      // unofficial type
      constexpr u8 base  = 0x52;
      constexpr u8 map[] = {72, 80, 96};
      u8 offset          = romSize() - base;
      GB_ASSERT(offset >= 0 && offset <= sizeof(map) / sizeof(map[0]));
      return 16 * map[offset];
    }
  }

  u8 ramSize() const {
    // 0x0149
    if (!inAnd(type(), kMBC1_RAM, kMBC1_RAM_BATTERY, kROM_RAM, kROM_RAM_BATTERY, kMMM01_RAM,
               kMMM01_RAM_BATTERY, kMBC3_TIMER_RAM_BATTERY, kMBC3_RAM, kMBC3_RAM_BATTERY, kMBC5_RAM,
               kMBC5_RAM_BATTERY, kMBC5_RUMBLE_RAM, kMBC5_RUMBLE_RAM_BATTERY, kMBC7_SENSOR_RUMBLE_RAM_BATTERY,
               kHuC1_RAM_BATTERY) &&
        rom_[0x149] == 0) {
      return 0;
    }
    return rom_[0x149];
  }

  u16 ramSizeByKB() const {
    constexpr u8 map[] = {0, 2, 8, 32, 128, 64};
    GB_ASSERT(ramSize() >= 0 && ramSize() <= sizeof(map) / sizeof(map[0]));
    return map[ramSize()];
  }

  u8 destinationCode() const {
    // 0x014A
    return rom_[0x14a];
  }

  u8 oldLicenseCode() const {
    // 0x014B
    return rom_[0x14b];
  }

  u8 maskROMVersion() const {
    // 0x014C
    return rom_[0x14c];
  }

  u8 headerChecksum() const {
    // 0x014D
    u8 checksum = 0;
    for (u16 address = 0x134; address <= 0x14c; address++) {
      checksum = checksum - rom_[address] - 1;
    }
    if (checksum != rom_[0x14d]) {
      GB_UNREACHABLE();
    }

    return rom_[0x14d];
  }

  u16 globalChecksum() const {
    // 0x014E-0x014F
    return rom_[0x14e] << 8 & rom_[0x14f];
  }

  u8 *rom() const { return rom_; }

private:
  constexpr inline static u8 nintendo_logo_[] = {
          0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
          0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
          0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
  };
  u8 *rom_{};
};

} // namespace gb
