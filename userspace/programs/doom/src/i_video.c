// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id:$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// $Log:$
//
// DESCRIPTION:
//	DOOM graphics stuff for X11, UNIX.
//
//-----------------------------------------------------------------------------

#include <deh_str.h>
#include <i_scale.h>
#include <w_wad.h>
static const char rcsid[] = "$Id: i_x.c,v 1.6 1997/02/03 22:45:10 b1 Exp $";

#include "config.h"
#include "d_event.h"
#include "d_main.h"
#include "i_system.h"
#include "i_video.h"
#include "m_argv.h"
#include "v_video.h"
#include "z_zone.h"

#include "doomkeys.h"
#include "tables.h"

#include "doomgeneric.h"

#include <stdbool.h>
#include <stdlib.h>

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include <stdarg.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

// #define CMAP256

struct FB_BitField {
    uint32_t offset; /* beginning of bitfield	*/
    uint32_t length; /* length of bitfield		*/
};

struct FB_ScreenInfo {
    uint32_t xres; /* visible resolution		*/
    uint32_t yres;
    uint32_t xres_virtual; /* virtual resolution		*/
    uint32_t yres_virtual;

    uint32_t bits_per_pixel; /* guess what			*/

    /* >1 = FOURCC			*/
    struct FB_BitField red;   /* bitfield in s_Fb mem if true color, */
    struct FB_BitField green; /* else only length is significant */
    struct FB_BitField blue;
    struct FB_BitField transp; /* transparency			*/
};

static struct FB_ScreenInfo s_Fb;
int usemouse = 0;

#ifdef CMAP256

boolean palette_changed;
struct color colors[256];

#else  // CMAP256

static struct color colors[256];

#endif  // CMAP256

void I_GetEvent(void);

// The screen buffer; this is modified to draw things to the screen

byte *I_VideoBuffer = NULL;

// If true, game is running as a screensaver

boolean screensaver_mode = false;

// Flag indicating whether the screen is currently visible:
// when the screen isnt visible, don't render the screen

boolean screenvisible;

// Mouse acceleration
//
// This emulates some of the behavior of DOS mouse drivers by increasing
// the speed when the mouse is moved fast.
//
// The mouse input values are input directly to the game, but when
// the values exceed the value of mouse_threshold, they are multiplied
// by mouse_acceleration to increase the speed.

float mouse_acceleration = 2.0;
int mouse_threshold      = 10;

// Gamma correction level to use

int usegamma = 0;

typedef struct {
    byte r;
    byte g;
    byte b;
} col_t;

// Palette converted to RGB565

static uint16_t rgb565_palette[256];

static unsigned int offsetX = 0;
static unsigned int offsetY = 0;

void I_InitGraphics(void)
{
    int i, gfxmodeparm;
    char *mode;

    memset(&s_Fb, 0, sizeof(struct FB_ScreenInfo));
    s_Fb.xres         = DG_ScreenMode->width;
    s_Fb.yres         = DG_ScreenMode->height;
    s_Fb.xres_virtual = s_Fb.xres;
    s_Fb.yres_virtual = s_Fb.yres;

#ifdef CMAP256

    s_Fb.bits_per_pixel = 8;

#else  // CMAP256

    gfxmodeparm = M_CheckParmWithArgs("-gfxmode", 1);

    if (gfxmodeparm) {
        mode = myargv[gfxmodeparm + 1];
    } else {
        // default to rgba8888 like the old behavior, for compatibility
        // maybe could warn here?
        mode = "rgba8888";
    }

    if (strcmp(mode, "rgba8888") == 0) {
        // default mode
        s_Fb.bits_per_pixel = 32;

        s_Fb.blue.length   = 8;
        s_Fb.green.length  = 8;
        s_Fb.red.length    = 8;
        s_Fb.transp.length = 8;

        s_Fb.blue.offset   = 0;
        s_Fb.green.offset  = 8;
        s_Fb.red.offset    = 16;
        s_Fb.transp.offset = 24;
    }

    else if (strcmp(mode, "rgb565") == 0) {
        s_Fb.bits_per_pixel = 16;

        s_Fb.blue.length   = 5;
        s_Fb.green.length  = 6;
        s_Fb.red.length    = 5;
        s_Fb.transp.length = 0;

        s_Fb.blue.offset   = 11;
        s_Fb.green.offset  = 5;
        s_Fb.red.offset    = 0;
        s_Fb.transp.offset = 16;
    } else
        I_Error("Unknown gfxmode value: %s\n", mode);

#endif  // CMAP256

    printf(
        "I_InitGraphics: framebuffer: x_res: %d, y_res: %d, x_virtual: %d, y_virtual: %d, bpp: "
        "%d\n",
        s_Fb.xres, s_Fb.yres, s_Fb.xres_virtual, s_Fb.yres_virtual, s_Fb.bits_per_pixel
    );

    printf(
        "I_InitGraphics: framebuffer: RGBA: %d%d%d%d, red_off: %d, green_off: %d, blue_off: %d, "
        "transp_off: %d\n",
        s_Fb.red.length, s_Fb.green.length, s_Fb.blue.length, s_Fb.transp.length, s_Fb.red.offset,
        s_Fb.green.offset, s_Fb.blue.offset, s_Fb.transp.offset
    );

    printf("I_InitGraphics: DOOM screen size: w x h: %d x %d\n", SCREENWIDTH, SCREENHEIGHT);

    /* Allocate screen to draw to */
    I_VideoBuffer =
        (byte *)Z_Malloc(SCREENWIDTH * SCREENHEIGHT, PU_STATIC, NULL);  // For DOOM to draw on

    screenvisible = true;

    extern void I_InitInput(void);
    I_InitInput();

    if (DG_ScreenWidth > (unsigned int)DG_ScreenMode->width) {
        offsetX = (DG_ScreenWidth - DG_ScreenMode->width) / 2;
    }
    if (DG_ScreenHeight > (unsigned int)DG_ScreenMode->height) {
        offsetY = (DG_ScreenHeight - DG_ScreenMode->height) / 2;
    }

    extern void I_InitScale(byte *, byte *, int);
    I_InitScale(I_VideoBuffer, (byte *)DG_ScreenBuffer, DG_ScreenMode->width);

    if (DG_ScreenMode->InitMode) {
        DG_ScreenMode->InitMode(W_CacheLumpName(DEH_String("PLAYPAL"), PU_CACHE));
    }
}

void I_ShutdownGraphics(void) { Z_Free(I_VideoBuffer); }

void I_StartFrame(void) {}

void I_StartTic(void) { I_GetEvent(); }

void I_UpdateNoBlit(void) {}

//
// I_FinishUpdate
//

void I_FinishUpdate(void)
{
    int i, k, stride;
    struct color c;
    uint16_t p16;
    uint32_t p32;
    unsigned char *line_in, *line_out;

    stride = s_Fb.bits_per_pixel / 8;

    /* DRAW SCREEN */
    line_in = (byte *)DG_ScreenBuffer;
    DG_ScreenMode->DrawScreen(0, 0, SCREENWIDTH, SCREENHEIGHT);

    line_out =
        line_in +
        (DG_ScreenHeight * DG_ScreenWidth - (offsetX + offsetY * DG_ScreenWidth) - 1) * stride;
    line_in += (DG_ScreenMode->width * DG_ScreenMode->height - 1);
    for (i = 0; i < DG_ScreenMode->height; i++) {
        for (k = 0; k < DG_ScreenMode->width; k++) {
            c = colors[*line_in];  // R:8 G:8 B:8

            if (s_Fb.bits_per_pixel == 16) {
                // RGB565 packing
                p16 = ((c.r & 0xF8) << 8) | ((c.g & 0xFC) << 3) | (c.b >> 3);

#ifdef SYS_BIG_ENDIAN
                p16 = swapeLE16(p16);  // can't use SHORT() because this needs to stay unsigned
#endif
                *(uint16_t *)line_out = p16;
                line_out -= sizeof(uint16_t);
            } else if (s_Fb.bits_per_pixel == 32) {
                // Assuming RGBA8888
                p32 = (c.r << s_Fb.red.offset) | (c.g << s_Fb.green.offset) |
                      (c.b << s_Fb.blue.offset);

#ifdef SYS_BIG_ENDIAN
                p32 = swapLE32(p32);
#endif
                *(uint32_t *)line_out = p32;
                line_out -= sizeof(uint32_t);
            } else {
                // no clue how to convert this
                I_Error("No idea how to convert %d bpp pixels", s_Fb.bits_per_pixel);
            }

            *line_in = 0;
            --line_in;
        }
        line_out -= 2 * offsetX * stride;
    }

    DG_DrawFrame();
}

//
// I_ReadScreen
//
void I_ReadScreen(byte *scr) { memcpy(scr, I_VideoBuffer, SCREENWIDTH * SCREENHEIGHT); }

//
// I_SetPalette
//
#define GFX_RGB565(r, g, b) \
    ((((r & 0xF8) >> 3) << 11) | (((g & 0xFC) >> 2) << 5) | ((b & 0xF8) >> 3))
#define GFX_RGB565_R(color) ((0xF800 & color) >> 11)
#define GFX_RGB565_G(color) ((0x07E0 & color) >> 5)
#define GFX_RGB565_B(color) (0x001F & color)

void I_SetPalette(byte *palette)
{
    int i;
    // col_t* c;

    // for (i = 0; i < 256; i++)
    //{
    //	c = (col_t*)palette;

    //	rgb565_palette[i] = GFX_RGB565(gammatable[usegamma][c->r],
    //								   gammatable[usegamma][c->g],
    //								   gammatable[usegamma][c->b]);

    //	palette += 3;
    //}

    /* performance boost:
     * map to the right pixel format over here! */

    I_ResetScaleTables(palette);

    for (i = 0; i < 256; ++i) {
        colors[i].a = 0;
        colors[i].r = gammatable[usegamma][*palette++];
        colors[i].g = gammatable[usegamma][*palette++];
        colors[i].b = gammatable[usegamma][*palette++];
    }

#ifdef CMAP256

    palette_changed = true;

#endif  // CMAP256
}

// Given an RGB value, find the closest matching palette index.

int I_GetPaletteIndex(int r, int g, int b)
{
    int best, best_diff, diff;
    int i;
    col_t color;

    printf("I_GetPaletteIndex\n");

    best      = 0;
    best_diff = INT_MAX;

    for (i = 0; i < 256; ++i) {
        color.r = GFX_RGB565_R(rgb565_palette[i]);
        color.g = GFX_RGB565_G(rgb565_palette[i]);
        color.b = GFX_RGB565_B(rgb565_palette[i]);

        diff = (r - color.r) * (r - color.r) + (g - color.g) * (g - color.g) +
               (b - color.b) * (b - color.b);

        if (diff < best_diff) {
            best      = i;
            best_diff = diff;
        }

        if (diff == 0) {
            break;
        }
    }

    return best;
}

void I_BeginRead(void) {}

void I_EndRead(void) {}

void I_SetWindowTitle(char *title) { DG_SetWindowTitle(title); }

void I_GraphicsCheckCommandLine(void) {}

void I_SetGrabMouseCallback(grabmouse_callback_t func) {}

void I_EnableLoadingDisk(void) {}

void I_BindVideoVariables(void) {}

void I_DisplayFPSDots(boolean dots_on) {}

void I_CheckIsScreensaver(void) {}
