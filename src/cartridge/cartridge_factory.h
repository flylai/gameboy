#pragma once

#include "cartridge.h"
#include "cartridge_header.h"
#include "mbc1.h"
#include "nameof.hpp"

namespace gb {

class CartridgeFactory {
public:
  static Cartridge *Create(const std::string &path) {
    std::ifstream is(path, std::ios::binary);
    if (!is.is_open()) {
      GB_UNREACHABLE();
    }
    is.seekg(0, std::ios::end);
    if (is.tellg() < CartridgeHeader::HEADER_LEN) {
      GB_LOG(ERROR) << "invalid cartridge file";
    }
    is.seekg(0);
    u8 header[CartridgeHeader::HEADER_LEN];
    is.read((char *) header, CartridgeHeader::HEADER_LEN);
    is.close();
    CartridgeHeader cart_header(header);
    switch (cart_header.type()) {
      case CartridgeHeader::kROM_ONLY:
      case CartridgeHeader::kMBC1:
      case CartridgeHeader::kMBC1_RAM:
      case CartridgeHeader::kMBC1_RAM_BATTERY:
        return new MBC1(path);
      case CartridgeHeader::kMBC2:
        goto unsupported;
        break;
      case CartridgeHeader::kMBC2_BATTERY:
        goto unsupported;
        break;
      case CartridgeHeader::kROM_RAM:
        goto unsupported;
        break;
      case CartridgeHeader::kROM_RAM_BATTERY:
        goto unsupported;
        break;
      case CartridgeHeader::kMMM01:
        goto unsupported;
        break;
      case CartridgeHeader::kMMM01_RAM:
        goto unsupported;
        break;
      case CartridgeHeader::kMMM01_RAM_BATTERY:
        goto unsupported;
        break;
      case CartridgeHeader::kMBC3_TIMER_BATTERY:
        goto unsupported;
        break;
      case CartridgeHeader::kMBC3_TIMER_RAM_BATTERY:
        goto unsupported;
        break;
      case CartridgeHeader::kMBC3:
        goto unsupported;
        break;
      case CartridgeHeader::kMBC3_RAM:
        goto unsupported;
        break;
      case CartridgeHeader::kMBC3_RAM_BATTERY:
        goto unsupported;
        break;
      case CartridgeHeader::kMBC5:
        goto unsupported;
        break;
      case CartridgeHeader::kMBC5_RAM:
        goto unsupported;
        break;
      case CartridgeHeader::kMBC5_RAM_BATTERY:
        goto unsupported;
        break;
      case CartridgeHeader::kMBC5_RUMBLE:
        goto unsupported;
        break;
      case CartridgeHeader::kMBC5_RUMBLE_RAM:
        goto unsupported;
        break;
      case CartridgeHeader::kMBC5_RUMBLE_RAM_BATTERY:
        goto unsupported;
        break;
      case CartridgeHeader::kMBC6:
        goto unsupported;
        break;
      case CartridgeHeader::kMBC7_SENSOR_RUMBLE_RAM_BATTERY:
        goto unsupported;
        break;
      case CartridgeHeader::kPOCKET_CAMERA:
        goto unsupported;
        break;
      case CartridgeHeader::kBANDAI_TAMA5:
        goto unsupported;
        break;
      case CartridgeHeader::kHuC3:
        goto unsupported;
        break;
      case CartridgeHeader::kHuC1_RAM_BATTERY:
        goto unsupported;
        break;
      default:
      unsupported:
        GB_LOG(ERROR) << "unsupported cartridge type " << NAMEOF_ENUM(cart_header.type());
    }
    GB_UNREACHABLE();
  }
};

} // namespace gb
