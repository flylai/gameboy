#include "timer.h"

#include "machine/cpu/interrupt.h"
#include "machine/memory/memory_bus.h"

namespace gb {

void Timer::tick() {
  if (tima_reload_counter_ > 0) {
    tima_reload_counter_--;
    if (tima_reload_counter_ == 0) {
      memory_bus_->if_.irq(InterruptType::kTIMER);
      TIMA(TMA());
    }
  }
  DIV(div_ + 1);
}

void Timer::DIV(u16 div) {
  u16 prev_div = div_;
  div_         = div;
  if (!enableTIMA()) {
    return;
  }

  static constexpr u8 bit_map[] = {9, 3, 5, 7};
  u8 clock                      = TAC() & 0x3;
  u8 prev_bit                   = (prev_div >> bit_map[clock]) & 0x1;
  u8 cur_bit                    = (div_ >> bit_map[clock]) & 0x1;

  if (prev_bit == 1 && cur_bit == 0 && !tima_reload_counter_) {
    TIMA(TIMA() + 1);
    if (TIMA() == 0) {
      tima_reload_counter_ = 4;
    }
  }
}

} // namespace gb
