#include "common/logger.h"
#include "machine/interrupt.h"
#include "machine/memory/memory_accessor.h"
#include "machine/ppu/ppu_register.h"
#include "machine/timer.h"

namespace gb {

// https://gbdev.io/pandocs/Memory_Map.html

class MemoryBus : public MemoryAccessor {
public:
  u8 get(u16 addr) const override { return getMemory(addr)->get(addr); }

  void set(u16 addr, u8 val) override { getMemory(addr)->set(addr, val); }

private:
  MemoryAccessor* getMemory(u16 addr) const {
    switch (addr) {
        // memory
      case 0x0000 ... 0x3FFF:
        // 16 KiB ROM bank 00 From cartridge, usually a fixed bank
        break;
      case 0x4000 ... 0x7FFF:
        // 16 KiB ROM Bank 01–NN From cartridge, switchable bank via mapper (if any)
        break;
      case 0x8000 ... 0x9FFF:
        // 8 KiB Video RAM (VRAM) In CGB mode, switchable bank 0/1
        return &vram_;
      case 0xA000 ... 0xBFFF:
        // 8 KiB External RAM From cartridge, switchable bank if any
        break;
      case 0xC000 ... 0xCFFF:
        // 4 KiB Work RAM (WRAM)
        return &wram_;
      case 0xD000 ... 0xDFFF:
        // 4 KiB Work RAM (WRAM) In CGB mode, switchable bank 1–7
        break;
      case 0xE000 ... 0xFDFF:
        // Echo RAM (mirror of C000–DDFF) Nintendo says use of this area is prohibited.
        break;
      case 0xFE00 ... 0xFE9F:
        // Object attribute memory (OAM)
        return &oam_;
      case 0xFEA0 ... 0xFEFF:
        // Not Usable Nintendo says use of this area is prohibited.
        break;
      case 0xFF80 ... 0xFFFE:
        // High RAM (HRAM)
        return &hram_;
      case 0xFFFF ... 0xFFFF:
        // Interrupt Enable register (IE)
        return &ie_;
      // io registers
      case 0xFF00:
        // Joypad input
        break;
      case 0xFF01 ... 0xFF02:
        //  Serial transfer
        break;
      case 0xFF04 ... 0xFF07:
        //  Timer and divider
        return timer_;
      case 0xFF0F:
        return &if_;
      case 0xFF10 ... 0xFF26:
        //  Audio
        break;
      case 0xFF30 ... 0xFF3F:
        //  Wave pattern
        break;
      case 0xFF40 ... 0xFF4B:
        //  LCD Control, Status, Position, Scrolling, and Palettes
        return ppu_registers_;
      case 0xFF4F:
        // VRAM Bank Select
        break;
      case 0xFF50:
        // Set to non-zero to disable boot ROM
        break;
      case 0xFF51 ... 0xFF55:
        // CGB VRAM DMA
        break;
      case 0xFF68 ... 0xFF6B:
        // CGB BG / OBJ Palettes
        break;
      case 0xFF70:
        // WRAM Bank Select
        break;
      default:
        GB_UNREACHABLE()
    }
    GB_UNREACHABLE()
  }

private:
  mutable Memory<0xc000, 0xdfff> wram_{};
  mutable Memory<0x8000, 0x9fff> vram_{};
  mutable Memory<0xfe00, 0xfe9f> oam_{};
  mutable Timer* timer_{};
  mutable InterruptFlag if_;
  mutable PPURegister* ppu_registers_;
  mutable Memory<0xff80, 0xfffe> hram_{};
  mutable InterruptEnable ie_;
};

} // namespace gb
