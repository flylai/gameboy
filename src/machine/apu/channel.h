#pragma once

#include "common/type.h"
#include "machine/apu/length_timer.h"
#include "machine/memory/memory_accessor.h"

namespace gb {


class Channel : public MemoryAccessor {
public:
  explicit Channel(u16 length = 64) : length_timer_(length) {}

  virtual ~Channel()          = default;

  virtual void tick()         = 0;

  virtual void trigger()      = 0;

  virtual bool enable() const = 0;

  virtual i16 output() const  = 0;

  // channel 123
  // channel 1 will override this
  virtual u16 period() const { return ((nrx4_ & 0x7) << 8) | nrx3_; }

  bool lengthEnable() const { return getBitN(nrx4_, 6); }

  void nrx3(u8 val) { nrx3_ = val; }

  void nrx4(u8 val) { nrx4_ = val; }

protected:
  bool channel_enable_{};
  LengthTimer length_timer_;
  u8 frame_sequencer_{};

  // some articles / code also name it frequency_timer.
  // nrx3, nrx4 providing the `period`
  u16 period_timer_{};

  u8 nrx3_{};
  u8 nrx4_{};
};

} // namespace gb
