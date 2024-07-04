#pragma once

#include "common/type.h"
#include "machine/memory/memory_accessor.h"

namespace gb {

class PPURegister : public Memory<0xff40, 0xff6b> {
public:
  PPURegister() { reset(); }

  void reset() {
    SCY(0x0);
    SCX(0x0);
    _LY(0x0);
    LYC(0x0);
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


#define DEF(V)                                 \
  V(u8, LCDC, 0xff40)                          \
  V(u8, STAT, 0xff41)                          \
  V(u8, SCY, 0xff42)                           \
  V(u8, SCX, 0xff43)                           \
  V(u8, _LY, 0xff44) /* we will overload it */ \
  V(u8, LYC, 0xff45)                           \
  V(u8, DMA, 0xff46)                           \
  V(u8, BGP, 0xff47)                           \
  V(u8, OBP0, 0xff48)                          \
  V(u8, OBP1, 0xff49)                          \
  V(u8, WY, 0xff4a)                            \
  V(u8, WX, 0xff4b)                            \
  V(u8, BCPS, 0xff68)                          \
  V(u8, BGPI, 0xff68)                          \
  V(u8, BCPD, 0xff69)                          \
  V(u8, BGPD, 0xff69)                          \
  V(u8, OCPS, 0xff6a)                          \
  V(u8, OBPI, 0xff6a)                          \
  V(u8, OCPD, 0xff6b)                          \
  V(u8, OBPD, 0xff6b)                          \
  V(u8, HDMA1, 0xff51)                         \
  V(u8, HDMA2, 0xff52)                         \
  V(u8, HDMA3, 0xff53)                         \
  V(u8, HDMA4, 0xff54)                         \
  V(u8, HDMA5, 0xff55)                         \
  V(u8, VBK, 0xff4f)


#define DEF_GET(TYPE, NAME, ADDR) \
  u8 NAME() const { return get(ADDR); }

#define DEF_SET(TYPE, NAME, ADDR) \
  void NAME(u8 val) { set(ADDR, val); }

  DEF(DEF_GET)
  DEF(DEF_SET)

  PPUMode mode() const { return static_cast<PPUMode>(STAT() & 0x3); }

  void mode(PPUMode val) { STAT(STAT() & 0xfc | static_cast<u8>(val)); }

  u8 LY() const { return _LY(); }

  void LY(u8 val) {
    _LY(val);
    if (LY() == LYC()) {
      // set lcd stat LYC=LY
      if (getBitN(STAT(), 6)) {
        STAT(setBitN(STAT(), 2));
      }
      // todo irq
    } else {
      STAT(clearBitN(STAT(), 2));
    }
  }

#undef DEF
#undef DEF_GET
#undef DEF_SET
};
} // namespace gb
