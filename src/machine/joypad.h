#include "machine/memory/memory_accessor.h"

namespace gb {
class Joypad : public Memory<0xff00, 0xff00> {};
} // namespace gb