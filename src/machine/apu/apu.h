#pragma once

#include <mutex>

#include "channel1.h"
#include "channel2.h"
#include "channel3.h"
#include "channel4.h"
#include "common/circle_buffer.h"
#include "global_register.h"
#include "machine/memory/memory_accessor.h"
#include "miniaudio.h"

/*
https://nightshade256.github.io/2021/03/27/gb-sound-emulation.html
https://www.callcc.dev/how-gameboy-apu-work
https://www.reddit.com/r/EmuDev/comments/5gkwi5/comment/dat3zni/
https://gbdev.io/pandocs/Audio_Registers.html
https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware
https://gbdev.io/pandocs/Audio_Registers.html
*/

namespace gb {

class APU : public MemoryAccessor {
public:
  APU() : apu_reg_(this) {}

  void tick();

  void audioDataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

  u8 get(u16 addr) const override;
  void set(u16 addr, u8 val) override;

  const Channel1& channel1() const { return channel1_; }

  const Channel2& channel2() const { return channel2_; }

  const Channel3& channel3() const { return channel3_; }

  const Channel4& channel4() const { return channel4_; }

private:
  APUGlobalRegister apu_reg_;
  u16 t_cycle_counter_{};

  Channel1 channel1_;
  Channel2 channel2_;
  Channel3 channel3_;
  Channel4 channel4_;

  CircleBuffer<i16> sample_buffer_{65535};
  u32 sample_buffer_counter_{};
  std::mutex sample_buffer_mutex_;
};

} // namespace gb
