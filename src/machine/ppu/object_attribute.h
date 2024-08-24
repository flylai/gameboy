#include "common/type.h"

namespace gb {
// OAM: 0xfe00-0xfe9f
// In OAM, each sprite occupies 4 bytes
struct ObjectAttribute {
  u8 y{};
  u8 x{};
  u8 tile_index{};
  u8 attrs{};
  // In Non-CGB mode, the smaller the X coordinate, the higher the priority.
  // When X coordinates are identical, the object located first in OAM has higher priority.
  u8 OAM_index{};

  bool operator<(const ObjectAttribute &o) const {
    if (x == o.x) {
      return OAM_index < o.OAM_index;
    }
    return x < o.x;
  }

  u8 priority() const { return attrs >> 7; }

  u8 yFlip() const { return attrs & 0x40; }

  u8 xFlip() const { return attrs & 0x20; }

  u8 dmgPalette() const { return attrs & 0x10; }

  u8 bank() const { return attrs & 0x8; }

  u8 cgbPalette() const { return attrs & 0x7; }
};
} // namespace gb
