#pragma once

#include "common/defs.h"
#include "common/type.h"
#include "machine/memory/memory_accessor.h"

namespace gb {

class MemoryBus;

class PPURegister : public Memory<0xff40, 0xff6b> {
public:
  PPURegister() { reset(); }

  void reset() {
    LCDC(0x91);
    STAT(0x81);
    SCY(0x0);
    SCX(0x0);
    LY(0x0);
    LYC(0x0);
    DMA(0xff);
    BGP(0xfc);
    OBP0(0xff);
    OBP1(0xff);
    WY(0x0);
    WX(0x0);
  }
  enum class PPUMode : u8 {
    kHORIZONTAL_BLANK = 0,
    kVERTICAL_BLANK,
    kOAM_SCAN,
    kDRAWING_PIXELS,
  };


#define DEF(V)             \
  V(u8, LCDC, LCDC_BASE)   \
  V(u8, STAT, STAT_BASE)   \
  V(u8, SCY, SCY_BASE)     \
  V(u8, SCX, SCX_BASE)     \
  V(u8, _LY, LY_BASE)      \
  V(u8, LYC, LYC_BASE)     \
  V(u8, DMA, DMA_BASE)     \
  V(u8, BGP, BGP_BASE)     \
  V(u8, OBP0, OBP0_BASE)   \
  V(u8, OBP1, OBP1_BASE)   \
  V(u8, WY, WY_BASE)       \
  V(u8, WX, WX_BASE)       \
  V(u8, BCPS, 0xff68)      \
  V(u8, BGPI, 0xff68)      \
  V(u8, BCPD, 0xff69)      \
  V(u8, BGPD, 0xff69)      \
  V(u8, OCPS, 0xff6a)      \
  V(u8, OBPI, 0xff6a)      \
  V(u8, OCPD, 0xff6b)      \
  V(u8, OBPD, 0xff6b)      \
  V(u8, HDMA1, HDMA1_BASE) \
  V(u8, HDMA2, HDMA2_BASE) \
  V(u8, HDMA3, HDMA3_BASE) \
  V(u8, HDMA4, HDMA4_BASE) \
  V(u8, HDMA5, HDMA5_BASE) \
  V(u8, VBK, VBK_BASE)


#define DEF_GET(TYPE, NAME, ADDR) \
  u8 NAME() const { return get(ADDR); }

#define DEF_SET(TYPE, NAME, ADDR) \
  void NAME(u8 val) { set(ADDR, val); }

  DEF(DEF_GET)
  DEF(DEF_SET)

  PPUMode mode() const { return static_cast<PPUMode>(STAT() & 0x3); }

  void mode(PPUMode val);

  void LY(u8 val);

  u8 LY() const { return _LY(); }

  void memoryBus(MemoryBus* memory_bus) { memory_bus_ = memory_bus; }

private:
  MemoryBus* memory_bus_{};

#undef DEF
#undef DEF_GET
#undef DEF_SET
};
} // namespace gb
