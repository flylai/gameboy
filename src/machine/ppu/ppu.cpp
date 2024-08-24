#include "ppu.h"

#include "machine/memory/memory_bus.h"

namespace gb {
// https://hacktix.github.io/GBEDG/ppu/

void PPU::dmaUpdate() {
  if (!dma_enable_) {
    dma_offset_ = 0;
    return;
  }
  if (dma_delay_) {
    dma_delay_--;
    return;
  }

  constexpr u16 OAM_BASE = 0xfe00;
  u16 dma_base           = memory_bus_->get(0xff46);

  u16 offset             = dma_base * 0x100 + dma_offset_;
  if (offset > 0xe000) {
    offset &= ~0x2000;
  }

  memory_bus_->set(OAM_BASE + dma_offset_, memory_bus_->get(offset));
  dma_enable_ = ++dma_offset_ <= 0x9f;
}

void PPU::increaseLY() {
  ppu_reg_.LY(ppu_reg_.LY() + 1);
  if (windowEnable() && windowVisible() //
      && ppu_reg_.LY() > ppu_reg_.WY()  //
      && ppu_reg_.LY() < (u16) (ppu_reg_.WY() + 144)) {
    fetcher_window_line_++;
  }
}

void PPU::horizontalBlank() {
  static constexpr u8 scx_dot[] = {
          204, 200, 200, 200, 200, 196, 196, 196,
  };
  if (dots_ == scx_dot[ppu_reg_.SCX() & 0x7]) {
    dots_ = 0;
    increaseLY();

    if (ppu_reg_.LY() == 144) {
      memory_bus_->if_.irq(InterruptType::kVBLANK);
      ppu_reg_.mode(PPURegister::PPUMode::kVERTICAL_BLANK);
    } else {
      ppu_reg_.mode(PPURegister::PPUMode::kOAM_SCAN);
    }
  }
}

void PPU::verticalBlank() {
  if (dots_ == 456) {
    dots_ = 0;
    increaseLY();
    if (ppu_reg_.LY() == 154) {
      ppu_reg_.mode(PPURegister::PPUMode::kOAM_SCAN);
      ppu_reg_.LY(0);
      fetcher_window_line_ = 0;
    }
  }
}

void PPU::oamScan() {
  if (dots_ == 80) {
    dots_ = 0;
    ppu_reg_.mode(PPURegister::PPUMode::kDRAWING_PIXELS);
  }
}

void PPU::drawingPixels() {
  if (dots_ == 172) {
    dots_ = 0;
    ppu_reg_.mode(PPURegister::PPUMode::kHORIZONTAL_BLANK);

    if (ppu_reg_.LY() > 144) {
      return;
    }


    std::fill(std::begin(scanline_rendered_), std::end(scanline_rendered_), 0);
    for (fetcher_x_ = 0; fetcher_x_ < 160; fetcher_x_++) {
      background_pixel_.reset();
      window_pixel_.reset();
      sprite_pixel_.reset();

      u16 background_addr = fetchBackgroundTile();
      fetchWindowOrBackgroundTileData(background_addr, false);


      u16 window_addr = fetchWindowTile();
      fetchWindowOrBackgroundTileData(window_addr, true);

      mixPixel();
    }
    if (objectEnable()) {
      fetchSprite();
      fetchAndDrawSpriteTileData();
    }
  }
}

inline u8 applyPalette(u8 color, u8 palette) {
  switch (color) {
    case 0:
      color = palette & 0x03;
      break;
    case 1:
      color = ((palette >> 2) & 0x03);
      break;
    case 2:
      color = ((palette >> 4) & 0x03);
      break;
    case 3:
      color = ((palette >> 6) & 0x03);
      break;
    default:
      GB_UNREACHABLE();
  }
  return color;
}

void setPixel(u8 *buffer, i32 x, i32 y, u8 r, u8 g, u8 b) {
  i32 offset         = (y * 160 + x) * 4;
  buffer[offset]     = r;
  buffer[offset + 1] = g;
  buffer[offset + 2] = b;
  buffer[offset + 3] = 0xff;
}

void PPU::mixPixel() {
  u8 *buffer = lcd_data_.get();
  u8 x       = fetcher_x_;
  u8 y       = ppu_reg_.LY();

  u8 bg_color{};
  u8 win_color{};

  bg_color = applyPalette(background_pixel_.color, background_pixel_.palette);
  setPixel(buffer, x, y, default_palette_[bg_color][0], default_palette_[bg_color][1],
           default_palette_[bg_color][2]);
  scanline_rendered_[x] = bg_color != 0;

  if (fetcher_x_ >= ppu_reg_.WX() - 7   //
      && windowVisible()                //
      && ppu_reg_.WY() <= ppu_reg_.LY() //
      && ppu_reg_.WX() <= 166) {
    win_color = applyPalette(window_pixel_.color, window_pixel_.palette);
    setPixel(buffer, x, y, default_palette_[win_color][0], default_palette_[win_color][1],
             default_palette_[win_color][2]);
    scanline_rendered_[x] = win_color != 0;
  }
}

u16 PPU::fetchWindowTile() {
  u16 addr      = windowTileBase();
  u16 x         = fetcher_x_ - (ppu_reg_.WX() - 7);
  u16 tile_x    = x >> 3;
  u16 y         = fetcher_window_line_;
  u16 tile_y    = y >> 3 << 5;
  u16 tile_addr = addr + tile_y + tile_x;
  u8 tile_idx   = memory_bus_->get(tile_addr);
  if (tileDataBase() == 0x8800) {
    tile_idx += 128;
  }
  u16 tile_data_addr = tileDataBase() + (tile_idx << 4) + ((y & 7) << 1);
  return tile_data_addr;
}

u16 PPU::fetchBackgroundTile() {
  u16 addr      = backgroundTileBase();
  u16 x         = fetcher_x_ + ppu_reg_.SCX();
  u16 tile_x    = (x >> 3) & 0x1f;
  u16 y         = ppu_reg_.LY() + ppu_reg_.SCY();
  u16 tile_y    = ((y >> 3) & 0x1f) << 5;
  u16 tile_addr = (addr & 0xfc00) + tile_y + tile_x;
  u8 tile_idx   = memory_bus_->get(tile_addr);
  if (tileDataBase() == 0x8800) {
    tile_idx += 128;
  }
  u16 tile_data_addr = (tileDataBase() + (tile_idx << 4)) + ((y & 7) << 1);
  return tile_data_addr;
}

void PPU::fetchWindowOrBackgroundTileData(u16 addr, bool is_window) {
  u8 low           = memory_bus_->get(addr);
  u8 high          = memory_bus_->get(addr + 1);

  u16 pixel_offset = is_window ? fetcher_x_ - (ppu_reg_.WX() - 7) : (fetcher_x_ + ppu_reg_.SCX());
  pixel_offset &= 0x7;

  Pixel &pixel = is_window ? window_pixel_ : background_pixel_;
  pixel.reset();
  if (backgroundWindowEnable()) {
    u8 color      = ((low << pixel_offset >> 7) & 1) | (((high << pixel_offset >> 7) & 1) << 1);
    pixel.color   = color;
    pixel.palette = ppu_reg_.BGP();
  } else {
    pixel.color   = 0;
    pixel.palette = 0;
  }
}

void PPU::fetchSprite() {
  while (!sprite_buffer_.empty()) {
    sprite_buffer_.pop();
  }

  for (u16 i = 0xfe00; i < 0xfea0; i += 4) {
    ObjectAttribute oa{};
    oa.y          = memory_bus_->get(i);
    oa.x          = memory_bus_->get(i + 1);
    oa.tile_index = memory_bus_->get(i + 2);
    oa.attrs      = memory_bus_->get(i + 3);
    oa.OAM_index  = i & 0xff;
    bool cond     = true;
    // Sprite X-Position must be greater than 0
    cond &= oa.x > 0;
    // LY + 16 must be greater than or equal to Sprite Y-Position
    cond &= ppu_reg_.LY() + 16 >= oa.y;
    // LY + 16 must be less than Sprite Y-Position + Sprite Height
    cond &= ppu_reg_.LY() + 16 < oa.y + objectHeight();
    if (cond) {
      sprite_buffer_.push(oa);
    }
  }
}

void PPU::fetchAndDrawSpriteTileData() {
  u8 sprite_height      = objectHeight();
  u8 sprite_height_mask = sprite_height == 16 ? 0xfe : 0xff;

  for (u8 i = 0; i <= 10 && !sprite_buffer_.empty(); i++) {
    ObjectAttribute oa = sprite_buffer_.top();

    u8 y               = ppu_reg_.LY() + 16 - oa.y;
    if (oa.yFlip()) {
      y = sprite_height - 1 - y;
    }

    for (u8 j = 0; j < 8; j++) {
      u8 x       = j;
      u8 pixel_x = oa.x - 8 + j;
      if (oa.priority() && scanline_rendered_[pixel_x] != 0) {
        continue;
      }

      if (oa.xFlip()) {
        x = 7 - x;
      }

      u16 tile_data_addr        = 0x8000 + ((oa.tile_index & sprite_height_mask) << 4) + (y << 1);
      u8 low                    = memory_bus_->get(tile_data_addr);
      u8 high                   = memory_bus_->get(tile_data_addr + 1);

      u8 color                  = ((low << x >> 7) & 1) | (((high << x >> 7) & 1) << 1);

      sprite_pixel_.color       = color;
      sprite_pixel_.palette     = oa.dmgPalette() ? ppu_reg_.OBP1() : ppu_reg_.OBP0();
      sprite_pixel_.bg_priority = oa.priority();

      u8 sprite_color           = applyPalette(sprite_pixel_.color, sprite_pixel_.palette);
      if (sprite_color) {
        setPixel(lcd_data_.get(), pixel_x, ppu_reg_.LY(), default_palette_[sprite_color][0],
                 default_palette_[sprite_color][1], default_palette_[sprite_color][2]);
      }
    }

    sprite_buffer_.pop();
  }
}

} // namespace gb
