#pragma once

#include <queue>

#include "common/type.h"
#include "machine/memory/memory_accessor.h"
#include "serial_buffer.h"

namespace gb {
class MemoryBus;

class Serial : public Memory<0xff01, 0xff02> {
public:
  Serial() {
    SB(0xff);
    SC(0x7c);
  }

  void tick();

  void SB(u8 val) { set(0xff01, val); }

  u8 SB() const { return get(0xff01); }

  void SC(u8 val) { set(0xff02, val); }

  u8 SC() const { return get(0xff02); }

  u8 enable() const { return getBitN(SC(), 7); }

  void enable(u8 val);

  u8 clockSpeed() const { return getBitN(SC(), 1); }

  void clockSpeed(u8 val) { SC(clearBitN(SC(), 1) | val); }

  // clock select 0-slave 1-master
  u8 clockSelect() const { return getBitN(SC(), 0); }

  void clockSelect(u8 val) { SC(clearBitN(SC(), 0) | val); }

  u8 dataReady() const { return clockSelect() == 1 ? SC() == 0x81 : SC() == 0x80; }

  void memoryBus(MemoryBus* memory_bus) { memory_bus_ = memory_bus; }

  void buffer(SerialBuffer* data) { data_ = data; }

private:
  MemoryBus* memory_bus_{};
  u8 buffer_{};
  u8 count_{};
  SerialBuffer* data_{};
};

} // namespace gb
