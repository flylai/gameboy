#include "joypad.h"

#include "machine/memory/memory_bus.h"

namespace gb {

#define DEF_KEY(NAME, TYPE, REG_BIT)                \
  void Joypad::NAME(bool press) {                   \
    if (!press) {                                   \
      TYPE##_ = setBitN(TYPE##_, REG_BIT);          \
    } else {                                        \
      TYPE##_ = clearBitN(TYPE##_, REG_BIT);        \
      memory_bus_->if_.irq(InterruptType::kJOYPAD); \
    }                                               \
  }

DEF_KEY(A, select, 0)
DEF_KEY(B, select, 1)
DEF_KEY(Select, select, 2)
DEF_KEY(Start, select, 3)
DEF_KEY(Right, direction, 0)
DEF_KEY(Left, direction, 1)
DEF_KEY(Up, direction, 2)
DEF_KEY(Down, direction, 3)

} // namespace gb
