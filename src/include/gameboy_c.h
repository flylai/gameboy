#pragma once

#include "common/utils.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef void *GameBoy;

GB_API GameBoy GameBoyInit(const char *game_path);
GB_API void GameBoyRun(GameBoy gb);
GB_API void GameBoyRunWithNewThread(GameBoy gb);
GB_API void GameBoyStop(GameBoy gb);
GB_API void GameBoyDestroy(GameBoy gb);
GB_API const char *GameBoyTextureBuffer(GameBoy gb);

GB_API void print(const char *msg);

#ifdef __cplusplus
}
#endif // __cplusplus
