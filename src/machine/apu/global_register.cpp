#include "global_register.h"

#include "apu.h"

namespace gb {

u8 APUGlobalRegister::get(u16 addr) const {
  if (addr == 0xff26) {
    u8 res = Memory::get(0xff26) & 0xf0;
    res |= apu_->channel1().enable() << 0;
    res |= apu_->channel2().enable() << 1;
    res |= apu_->channel3().enable() << 2;
    res |= apu_->channel4().enable() << 3;
    return res;
  } else {
    return Memory::get(addr);
  }
}

} // namespace gb
