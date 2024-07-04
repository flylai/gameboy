#include "ppu_register.h"

#include "machine/memory/memory_bus.h"

namespace gb {
void PPURegister::LY(u8 val) {
  _LY(val);
  if (LY() == LYC()) {
    // set lcd stat LYC=LY
    if (getBitN(STAT(), 6)) {
      STAT(setBitN(STAT(), 2));
    }
    memory_bus_->if_.irq(InterruptType::kLCD);
  } else {
    STAT(clearBitN(STAT(), 2));
  }
}
} // namespace gb
