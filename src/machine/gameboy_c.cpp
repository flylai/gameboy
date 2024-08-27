#include "include/gameboy_c.h"

#include "gameboy.h"

#define CHECK_GB(GB)      \
  if (!GB) [[unlikely]] { \
    return;               \
  }

extern "C" GameBoy GameBoyInit(const char *game_path) { return new gb::GameBoy(game_path); }

extern "C" void GameBoyRun(GameBoy gb) {
  CHECK_GB(gb)
  auto *gameboy = (gb::GameBoy *) gb;
  gameboy->rtc_.run();
}

extern "C" void GameBoyRunWithNewThread(GameBoy gb) {
  CHECK_GB(gb)
  auto *gameboy = (gb::GameBoy *) gb;
  std::thread t([=]() { gameboy->rtc_.run(); });
  t.detach();
}

extern "C" void GameBoyStop(GameBoy gb) {
  CHECK_GB(gb)
  auto *gameboy = (gb::GameBoy *) gb;
  gameboy->rtc_.stop();
}

extern "C" void GameBoyDestroy(GameBoy gb) {
  CHECK_GB(gb)
  auto *gameboy = (gb::GameBoy *) gb;
  gameboy->rtc_.stop();
}

extern "C" const char *GameBoyTextureBuffer(GameBoy gb) {
  if (!gb) [[unlikely]] {
    return nullptr;
  }
  auto *gameboy = (gb::GameBoy *) gb;
  return (char *) gameboy->ppu_.lcdData().get();
}
