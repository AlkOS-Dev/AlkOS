#ifndef USERSPACE_PROGRAMS_DOOM_SRC_DOOMGENERIC_H_
#define USERSPACE_PROGRAMS_DOOM_SRC_DOOMGENERIC_H_

#include "i_video.h"

#include <stdint.h>
#include <stdlib.h>

#ifdef CMAP256

typedef uint8_t pixel_t;

#else  // CMAP256

typedef uint32_t pixel_t;

#endif  // CMAP256

// Global screen mode and buffer
extern screen_mode_t *DG_ScreenMode;
extern pixel_t *DG_ScreenBuffer;
extern int DG_ScreenWidth;
extern int DG_ScreenHeight;

#ifdef __cplusplus
extern "C" {
#endif

void doomgeneric_Create(int argc, char **argv);
void doomgeneric_Tick();

// Implement below functions for your platform
void DG_Init();
void DG_DrawFrame();
void DG_SleepMs(uint32_t ms);
uint32_t DG_GetTicksMs();
int DG_GetKey(int *pressed, unsigned char *key);
void DG_SetWindowTitle(const char *title);

#ifdef __cplusplus
}
#endif

#endif  // USERSPACE_PROGRAMS_DOOM_SRC_DOOMGENERIC_H_
