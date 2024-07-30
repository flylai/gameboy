#include "ppu_register.h"

#include "machine/memory/memory_bus.h"

namespace gb {

void PPURegister::mode(PPUMode val) {
  STAT((STAT() & 0xfc) | static_cast<u8>(val));
  if (val == PPUMode::kHORIZONTAL_BLANK && getBitN(STAT(), 3)) {
    memory_bus_->if_.irq(InterruptType::kLCDSTAT);
  } else if (val == PPUMode::kVERTICAL_BLANK && getBitN(STAT(), 4)) {
    memory_bus_->if_.irq(InterruptType::kLCDSTAT);
  } else if (val == PPUMode::kOAM_SCAN && getBitN(STAT(), 5)) {
    memory_bus_->if_.irq(InterruptType::kLCDSTAT);
  }
}

void PPURegister::LY(u8 val) {
  _LY(val);
  if (_LY() == LYC()) {
    STAT(setBitN(STAT(), 2));
    if (getBitN(STAT(), 6)) {
      memory_bus_->if_.irq(InterruptType::kLCDSTAT);
    }
  } else {
    STAT(clearBitN(STAT(), 2));
  }
}

} // namespace gb
