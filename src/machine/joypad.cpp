#include "joypad.h"

#include "machine/memory/memory_bus.h"

namespace gb {

// A / Right
void Joypad::p1_0_set() {
  ram_[0] = clearBitN(ram_[0], 0);
  memory_bus_->if_.irq(InterruptType::kJOYPAD);
}

void Joypad::p1_0_clear() { ram_[0] = setBitN(ram_[0], 0); }

// B / Left
void Joypad::p1_1_set() {
  ram_[0] = clearBitN(ram_[0], 1);
  memory_bus_->if_.irq(InterruptType::kJOYPAD);
}

void Joypad::p1_1_clear() { ram_[0] = setBitN(ram_[0], 1); }

// Select / Up
void Joypad::p1_2_set() {
  ram_[0] = clearBitN(ram_[0], 2);
  memory_bus_->if_.irq(InterruptType::kJOYPAD);
}

void Joypad::p1_2_clear() { ram_[0] = setBitN(ram_[0], 2); }

// Start / Down
void Joypad::p1_3_set() {
  ram_[0] = clearBitN(ram_[0], 3);
  memory_bus_->if_.irq(InterruptType::kJOYPAD);
}

void Joypad::p1_3_clear() { ram_[0] = setBitN(ram_[0], 3); }


} // namespace gb
