#include "timer.h"

#include "interrupt.h"
#include "machine/memory/memory_bus.h"

namespace gb {

void Timer::increaseTIMA() {
  if (!enableTIMA()) {
    return;
  }
  TIMA(TIMA() + 1);
  if (TIMA() == 0x00) {
    // overflow
    TIMA(TMA());
    memory_bus_->if_.irq(InterruptType::kTIMER);
  }
}

} // namespace gb
