#include "miniaudio_wrapper.h"

#include "machine/gameboy.h"

// clang-format off
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
// clang-format on

namespace gb {

MiniAudioWrapper::~MiniAudioWrapper() { ma_device_uninit(&device); }

void MiniAudioWrapper::init() {
  device_config                   = ma_device_config_init(ma_device_type_playback);
  device_config.playback.format   = ma_format_s16;
  device_config.playback.channels = 2;
  device_config.sampleRate        = ma_standard_sample_rate_44100;
  device_config.dataCallback      = data_callback;
  device_config.pUserData         = gb;

  result                          = ma_device_init(nullptr, &device_config, &device);

  if (result != MA_SUCCESS) {
    GB_LOG(INFO) << "Failed to initialize playback device.";
  }

  result = ma_device_start(&device);
  if (result != MA_SUCCESS) {
    GB_LOG(INFO) << "Failed to start playback device.";
    ma_device_uninit(&device);
  }
}

void MiniAudioWrapper::data_callback(ma_device* device, void* output, const void* input,
                                     ma_uint32 frame_count) {
  auto* gb = static_cast<GameBoy*>(device->pUserData);
  if (!gb) [[unlikely]] {
    return;
  }
  gb->apu_.audioDataCallback(device, output, input, frame_count);
}

} // namespace gb
