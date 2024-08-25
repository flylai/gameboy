#pragma once

#include "cartridge/cartridge.h"
#include "cartridge/cartridge_factory.h"
#include "machine/apu/apu.h"
#include "machine/apu/miniaudio_wrapper.h"
#include "machine/cpu/cpu.h"
#include "machine/cpu/rtc.h"
#include "machine/cpu/timer.h"
#include "machine/ppu/ppu.h"

namespace gb {

class GameBoy {
public:
  ~GameBoy() { delete cartridge_; }

  explicit GameBoy(const std::string& game)
      : cartridge_(CartridgeFactory::Create(game)),
        miniaudio_wrapper(this) {
    memory_bus_.timer_     = &timer_;
    memory_bus_.cartridge_ = cartridge_;
    memory_bus_.ppu_       = &ppu_;
    memory_bus_.serial_    = &serial_;
    memory_bus_.joypad_    = &joypad_;
    memory_bus_.apu_       = &apu_;
    cpu_.memoryBus(&memory_bus_);
    ppu_.memoryBus(&memory_bus_);
    timer_.memoryBus(&memory_bus_);
    serial_.memoryBus(&memory_bus_);
    joypad_.memoryBus(&memory_bus_);
    rtc_.addTask([this]() { return cpu_.update(); });

    cpu_.reset();
    memory_bus_.reset();

    miniaudio_wrapper.init();
  }

  Cartridge* cartridge_{};
  RTC rtc_;
  Timer timer_;
  CPU cpu_;
  Serial serial_;
  MemoryBus memory_bus_;
  PPU ppu_;
  Joypad joypad_;
  APU apu_;
  MiniAudioWrapper miniaudio_wrapper;
};


} // namespace gb
