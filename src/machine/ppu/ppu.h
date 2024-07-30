#pragma once

#include <array>
#include <queue>

#include "common/logger.h"
#include "common/type.h"
#include "machine/memory/memory_accessor.h"
#include "object_attribute.h"
#include "ppu_register.h"

namespace gb {
class MemoryBus;

class LCDData {
public:
  u8 *get() { return ram_; }

  const u8 *get() const { return ram_; }

private:
  u8 ram_[160 * 144 * 4 /*rgba*/];
};

class PPU : public MemoryAccessor {
  struct Pixel {
    u8 color{};   // 2 bit
    u8 palette{}; //3 bit
    u8 sprite_priority{};
    u8 bg_priority{};

    void reset() {
      color           = 0;
      palette         = 0;
      sprite_priority = 0;
      bg_priority     = 0;
    }
  };

  u8 fetcher_x_{};
  Pixel background_pixel_;
  Pixel window_pixel_;
  Pixel sprite_pixel_;

public:
  u8 get(u16 addr) const override { return ppu_reg_.get(addr); }

  void set(u16 addr, u8 val) override {
    if (addr == 0xff46) {
      dma_enable_ = true;
      dma_delay_  = 1;
    }
    ppu_reg_.set(addr, val);
  }

  void tick() {
    if (!getBitN(ppu_reg_.LCDC(), 7)) {
      GB_LOG(DEBUG) << "LCD/PPU is off";
      return;
    }
    dmaUpdate();
    dots_++;
    switch (ppu_reg_.mode()) {
      case PPURegister::PPUMode::kHORIZONTAL_BLANK:
        horizontalBlank();
        break;
      case PPURegister::PPUMode::kVERTICAL_BLANK:
        verticalBlank();
        break;
      case PPURegister::PPUMode::kOAM_SCAN:
        oamScan();
        break;
      case PPURegister::PPUMode::kDRAWING_PIXELS:
        drawingPixels();
        break;
      default:
        GB_UNREACHABLE();
    }
  }

  const LCDData &lcdData() const { return lcd_data_; }

  void memoryBus(MemoryBus *memory_bus) {
    memory_bus_ = memory_bus;
    ppu_reg_.memoryBus(memory_bus);
  }

private:
  void dmaUpdate();
  void horizontalBlank();
  void verticalBlank();
  void oamScan();
  void drawingPixels();

  bool windowEnable() const { return getBitN(ppu_reg_.LCDC(), 5); }

  bool windowVisible() const { return windowEnable() && ppu_reg_.WX() <= 166 && ppu_reg_.WY() <= 143; }

  bool objectEnable() const { return getBitN(ppu_reg_.LCDC(), 1); }

  u8 objectHeight() const { return getBitN(ppu_reg_.LCDC(), 2) ? 16 : 8; }

  bool backgroundWindowEnable() const { return getBitN(ppu_reg_.LCDC(), 0); }

  u16 windowTileBase() const { return getBitN(ppu_reg_.LCDC(), 6) ? 0x9c00 : 0x9800; }

  u16 backgroundTileBase() const { return getBitN(ppu_reg_.LCDC(), 3) ? 0x9c00 : 0x9800; }

  u16 tileDataBase() const { return getBitN(ppu_reg_.LCDC(), 4) ? 0x8000 : 0x8800; }

  u16 fetchWindowTile();
  u16 fetchBackgroundTile();
  void fetchWindowOrBackgroundTileData(u16 addr, bool is_window);
  void fetchSprite();
  void fetchSpriteTileData();
  void mixPixel();

  void increaseLY();

private:
  bool dma_enable_{};
  u16 dma_offset_{};
  u8 dma_delay_{};

  PPURegister ppu_reg_;
  MemoryBus *memory_bus_{};
  LCDData lcd_data_;
  std::priority_queue<ObjectAttribute> sprite_buffer_;
  std::vector<ObjectAttribute> fetched_sprites_;
  u8 fetcher_window_line_{};
  u16 dots_{};

  static constexpr u8 default_palette_[][3] = {
          {153, 161, 120},
          {87, 93, 67},
          {42, 46, 32},
          {10, 10, 2},
  };
};

} // namespace gb
