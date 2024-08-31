#include "disassembler.h"

#include "common/defs.h"
#include "machine/memory/memory_bus.h"

namespace gb {


void Disassembler::disassembleROM() {
  u32 rom_size = memory_bus_->cartridge_->header().romSizeByKB() * 1024;

  // decode 0x0000 - 0x7fff only.
  // if some executable code between 0x8000 - 0xffff
  // using disassemble(address) to decode it.
  // TODO: if the ROM BANK changed, we need to decode 0x4000-0x7fff again.
  if (rom_size > ROM_BANK_N_BASE) {
    rom_size = VRAM_BASE;
  }

  for (u16 i = 0; i < rom_size;) {
    i += disassemble(i, true);
  }
}

u8 Disassembler::disassemble(u16 pc, bool force_update) {
  if (!enable_ && !force_update) {
    return 0;
  }

  if (pc < ROM_BANK_N_BASE && !force_update) [[likely]] {
    return 0;
  }

  auto& record = records_[pc];
  record.pc    = pc;

  if (record.instruction == nullptr) {
    records_count_++;
  }

  u8 inst_idx        = memory_bus_->get(pc++);
  record.instruction = &instructions_unprefixed_[inst_idx];
  if (inst_idx == 0xcb) {
    u8 cb_idx          = memory_bus_->get(pc++);
    record.instruction = &instruction_prefixed_[cb_idx];
  }

  for (int i = 0; i < record.instruction->size - 1 - (inst_idx == 0xcb); i++) {
    record.operand[i] = memory_bus_->get(pc++);
  }
  return record.instruction->size;
}

} // namespace gb
