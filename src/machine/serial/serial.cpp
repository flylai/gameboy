#include "serial.h"

#include "machine/memory/memory_bus.h"

namespace gb {
u8 Serial::update(u64 cycle) {
  if (enable() && count_) {
    SB(SB() << 1 | 1);
    if (--count_ == 0) {
      enable(false);
      GB_LOG(INFO) << buffer_;
      if (data_queue_ != nullptr) {
        data_queue_->push(buffer_);
      }
    }
  } else {
    buffer_ = SB();
    count_  = 8;
  }
  return 0;
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
