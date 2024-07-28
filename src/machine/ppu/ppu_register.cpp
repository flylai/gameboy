#include "ppu_register.h"

#include "machine/memory/memory_bus.h"

namespace gb {

void PPURegister::mode(PPUMode val) {
  STAT(STAT() & 0xfc | static_cast<u8>(val));
  if (val == PPUMode::kHORIZONTAL_BLANK && getBitN(STAT(), 3)) {
    memory_bus_->if_.irq(InterruptType::kLCDSTAT);
  } else if (val == PPUMode::kVERTICAL_BLANK && getBitN(STAT(), 4)) {
    memory_bus_->if_.irq(InterruptType::kVBLANK);
    memory_bus_->if_.irq(InterruptType::kLCDSTAT);
  } else if (val == PPUMode::kOAM_SCAN && getBitN(STAT(), 5)) {
    memory_bus_->if_.irq(InterruptType::kLCDSTAT);
  }
}

} // namespace gb
