#pragma once

#include "common/logger.h"
#include "miniaudio.h"

namespace gb {

class GameBoy;

struct MiniAudioWrapper {
  explicit MiniAudioWrapper(GameBoy* gb) : gb(gb) {}

  ~MiniAudioWrapper();

  void init();

  static void data_callback(ma_device* device, void* output, const void* input, ma_uint32 frame_count);

  GameBoy* gb;
  ma_result result;
  ma_device_config device_config{};
  ma_device device{};
};

} // namespace gb
