#include "serial.h"

#include "machine/memory/memory_bus.h"

namespace gb {
void Serial::tick() {
  if (enable() && count_) {
    SB(SB() << 1 | 1);
    if (--count_ == 0) {
      enable(false);
#if defined(GB_TEST) || !defined(NDEBUG)
      if (isprint(buffer_)) {
        GB_LOG(INFO) << buffer_;
      } else {
        GB_LOG(INFO) << "0x" << std::hex << (u32) buffer_;
      }
#endif // defined(GB_TEST) || defined(NDEBUG)
      if (data_ != nullptr) {
        data_->push(buffer_);
      }
    }
  } else {
    buffer_ = SB();
    count_  = 8;
  }
}

void Serial::enable(u8 val) {
  if (val == 0) {
    if (getBitN(SC(), 7) == 1) {
      memory_bus_->if_.irq(InterruptType::kSERIAL);
    }
    SC(clearBitN(SC(), 7));
  } else {
    SC(setBitN(SC(), 7));
  }
}


} // namespace gb
