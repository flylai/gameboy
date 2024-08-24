#include "cartridge/cartridge.h"
#include "common/logger.h"
#include "machine/apu/apu.h"
#include "machine/cpu/interrupt.h"
#include "machine/cpu/timer.h"
#include "machine/joypad.h"
#include "machine/memory/memory_accessor.h"
#include "machine/ppu/ppu.h"
#include "machine/serial/serial.h"
#include "work_ram.h"

namespace gb {

// https://gbdev.io/pandocs/Memory_Map.html

class MemoryBus : public MemoryAccessor {
  class InvalidMemory : public Memory<0, 0> {
  public:
    u8 get(u16 addr) const override { return 0xff; }

    void set(u16 addr, u8 val) override {}
  };

public:
  void reset() {
    setWithoutCheck(0xFF00, 0xCF); //P1
    setWithoutCheck(0xFF01, 0x00); //SB
    setWithoutCheck(0xFF02, 0x7E); //SC
    setWithoutCheck(0xFF04, 0xAB); //DIV
    setWithoutCheck(0xFF05, 0x00); //TIMA
    setWithoutCheck(0xFF06, 0x00); //TMA
    setWithoutCheck(0xFF07, 0xF8); //TAC
    setWithoutCheck(0xFF0F, 0xE1); //IF
    setWithoutCheck(0xFF10, 0x80); //NR10
    setWithoutCheck(0xFF11, 0xBF); //NR11
    setWithoutCheck(0xFF12, 0xF3); //NR12
    setWithoutCheck(0xFF13, 0xFF); //NR13
    setWithoutCheck(0xFF14, 0xBF); //NR14
    setWithoutCheck(0xFF16, 0x3F); //NR21
    setWithoutCheck(0xFF17, 0x00); //NR22
    setWithoutCheck(0xFF18, 0xFF); //NR23
    setWithoutCheck(0xFF19, 0xBF); //NR24
    setWithoutCheck(0xFF1A, 0x7F); //NR30
    setWithoutCheck(0xFF1B, 0xFF); //NR31
    setWithoutCheck(0xFF1C, 0x9F); //NR32
    setWithoutCheck(0xFF1D, 0xFF); //NR33
    setWithoutCheck(0xFF1E, 0xBF); //NR34
    setWithoutCheck(0xFF20, 0xFF); //NR41
    setWithoutCheck(0xFF21, 0x00); //NR42
    setWithoutCheck(0xFF22, 0x00); //NR43
    setWithoutCheck(0xFF23, 0xBF); //NR44
    setWithoutCheck(0xFF24, 0x77); //NR50
    setWithoutCheck(0xFF25, 0xF3); //NR51
    setWithoutCheck(0xFF26, 0xF1); //NR52
    setWithoutCheck(0xFF40, 0x91); //LCDC
    setWithoutCheck(0xFF41, 0x85); //STAT
    setWithoutCheck(0xFF42, 0x00); //SCY
    setWithoutCheck(0xFF43, 0x00); //SCX
    setWithoutCheck(0xFF44, 0x00); //LY
    setWithoutCheck(0xFF45, 0x00); //LYC
    setWithoutCheck(0xFF46, 0xFF); //DMA
    setWithoutCheck(0xFF47, 0xFC); //BGP
    setWithoutCheck(0xFF48, 0x07); //OBP0
    setWithoutCheck(0xFF49, 0x07); //OBP1
    setWithoutCheck(0xFF4A, 0x00); //WY
    setWithoutCheck(0xFF4B, 0x00); //WX
    setWithoutCheck(0xFFFF, 0x00); //IE
  }

  u8 get(u16 addr) const override {
    if (accessType(addr) == AccessType::kW) [[unlikely]] {
      return 0xff;
    }
    return getMemory(addr)->get(addr);
  }

  void set(u16 addr, u8 val) override {
    if (accessType(addr) == AccessType::kR) [[unlikely]] {
      return;
    }
    getMemory(addr)->set(addr, val);
  }

  INLINE void setWithoutCheck(u16 addr, u8 val) { getMemory(addr)->set(addr, val); }

  void tick() const {
    for (u8 i = 0; i < 4; i++) {
      timer_->tick();
      serial_->tick();
      ppu_->tick();
      apu_->tick();
    }
  }

private:
  enum class AccessType : u8 {
    kW,
    kR,
    kRW,
    kMIXED,
  };

  MemoryAccessor *getMemory(u16 addr) const {
    switch (addr) {
        // memory
      case 0x0000 ... 0x3FFF:
        // 16 KiB ROM bank 00 From cartridge, usually a fixed bank
        return cartridge_;
      case 0x4000 ... 0x7FFF:
        // 16 KiB ROM Bank 01–NN From cartridge, switchable bank via mapper (if any)
        return cartridge_;
      case 0x8000 ... 0x9FFF:
        // 8 KiB Video RAM (VRAM) In CGB mode, switchable bank 0/1
        return &vram_;
      case 0xA000 ... 0xBFFF:
        // 8 KiB External RAM From cartridge, switchable bank if any
        return cartridge_;
      case 0xC000 ... 0xCFFF:
        // 4 KiB Work RAM (WRAM)
        return &wram_;
      case 0xD000 ... 0xDFFF:
        // 4 KiB Work RAM (WRAM) In CGB mode, switchable bank 1–7
        return &wram_;
      case 0xE000 ... 0xFDFF:
        // Echo RAM (mirror of C000–DDFF) Nintendo says use of this area is prohibited.
        return &wram_;
      case 0xFE00 ... 0xFE9F:
        // Object attribute memory (OAM)
        return ppu_;
      case 0xFEA0 ... 0xFEFF:
        // Not Usable Nintendo says use of this area is prohibited.
        goto invalid_addr;
      case 0xFF80 ... 0xFFFE:
        // High RAM (HRAM)
        return &hram_;
      case 0xFFFF:
        // Interrupt Enable register (IE)
        return &ie_;
      // io registers
      case 0xFF00:
        // Joypad input
        return joypad_;
      case 0xFF01 ... 0xFF02:
        return serial_;
      case 0xFF04 ... 0xFF07:
        //  Timer and divider
        return timer_;
      case 0xFF0F:
        return &if_;
      case 0xFF10 ... 0xFF26:
        //  Audio
        return apu_;
      case 0xFF30 ... 0xFF3F:
        //  Wave pattern
        return apu_;
      case 0xFF40 ... 0xFF4B:
        //  LCD Control, Status, Position, Scrolling, and Palettes
        return ppu_;
      case 0xFF4D:
        return &speed_switch_;
      case 0xFF4F:
        // VRAM Bank Select
        goto invalid_addr;
      case 0xFF50:
        // Set to non-zero to disable boot ROM
        goto invalid_addr;
      case 0xFF51 ... 0xFF55:
        // CGB VRAM DMA
        goto invalid_addr;
      case 0xFF68 ... 0xFF6B:
        // CGB BG / OBJ Palettes
        goto invalid_addr;
      case 0xFF70:
        // WRAM Bank Select
        return &wram_;
      default:
      invalid_addr:
        //        GB_LOG(WARN) << "try to access addr " << std::hex << addr << " is invalid_addr";
        return &invalid_memory_;
    }
    GB_UNREACHABLE()
  }

  static inline AccessType accessType(u16 addr) {
    static const std::unordered_map<u16, AccessType> m{
            {0xFF00, AccessType::kMIXED}, {0xFF01, AccessType::kRW},    {0xFF02, AccessType::kRW},
            {0xFF04, AccessType::kRW},    {0xFF05, AccessType::kRW},    {0xFF06, AccessType::kRW},
            {0xFF07, AccessType::kRW},    {0xFF0F, AccessType::kRW},    {0xFF10, AccessType::kRW},
            {0xFF11, AccessType::kMIXED}, {0xFF12, AccessType::kRW},    {0xFF13, AccessType::kW},
            {0xFF14, AccessType::kMIXED}, {0xFF16, AccessType::kMIXED}, {0xFF17, AccessType::kRW},
            {0xFF18, AccessType::kW},     {0xFF19, AccessType::kMIXED}, {0xFF1A, AccessType::kRW},
            {0xFF1B, AccessType::kW},     {0xFF1C, AccessType::kRW},    {0xFF1D, AccessType::kW},
            {0xFF1E, AccessType::kMIXED}, {0xFF20, AccessType::kW},     {0xFF21, AccessType::kRW},
            {0xFF22, AccessType::kRW},    {0xFF23, AccessType::kMIXED}, {0xFF24, AccessType::kRW},
            {0xFF25, AccessType::kRW},    {0xFF26, AccessType::kMIXED}, {0xFF30, AccessType::kRW},
            {0xFF40, AccessType::kRW},    {0xFF41, AccessType::kMIXED}, {0xFF42, AccessType::kRW},
            {0xFF43, AccessType::kRW},    {0xFF44, AccessType::kR},     {0xFF45, AccessType::kRW},
            {0xFF46, AccessType::kRW},    {0xFF47, AccessType::kRW},    {0xFF48, AccessType::kRW},
            {0xFF49, AccessType::kRW},    {0xFF4A, AccessType::kRW},    {0xFF4B, AccessType::kRW},
            {0xFF4D, AccessType::kMIXED}, {0xFF4F, AccessType::kRW},    {0xFF51, AccessType::kW},
            {0xFF52, AccessType::kW},     {0xFF53, AccessType::kW},     {0xFF54, AccessType::kW},
            {0xFF55, AccessType::kRW},    {0xFF56, AccessType::kMIXED}, {0xFF68, AccessType::kRW},
            {0xFF69, AccessType::kRW},    {0xFF6A, AccessType::kRW},    {0xFF6B, AccessType::kRW},
            {0xFF6C, AccessType::kRW},    {0xFF70, AccessType::kRW},    {0xFF76, AccessType::kR},
            {0xFF77, AccessType::kR},     {0xFFFF, AccessType::kRW},
    };
    if (auto it = m.find(addr); it != m.end()) {
      return it->second;
    }
    return AccessType::kRW;
  }

public:
  mutable Cartridge *cartridge_{};
  mutable WorkRam wram_{};
  mutable Memory<0x8000, 0x9fff> vram_{};

  mutable Joypad *joypad_{};
  mutable Serial *serial_{};
  mutable Timer *timer_{};
  mutable InterruptFlag if_;
  mutable APU *apu_{};
  mutable PPU *ppu_{};
  mutable Memory<0xff4d, 0xff4d> speed_switch_{};
  mutable Memory<0xff80, 0xfffe> hram_{};
  mutable InterruptEnable ie_;
  mutable InvalidMemory invalid_memory_;
};

} // namespace gb
