#include "timer.h"

#include "interrupt.h"
#include "machine/memory/memory_bus.h"

namespace gb {

void Timer::increaseTIMA() {
  if (!enable()) {
    return;
  }
  _TIMA(_TIMA() + 1);
  if (_TIMA() == 0) {
    // overflow
    _TIMA(TMA());
    memory_bus_->if_.irq(InterruptType::kTIMER);
  }
}

} // namespace gb