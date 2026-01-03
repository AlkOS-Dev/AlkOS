#ifndef LIBS_LIBC_SRC_INCLUDE_ALKOS_VIDEO_H_
#define LIBS_LIBC_SRC_INCLUDE_ALKOS_VIDEO_H_

#include <types.h>

typedef struct {
    i32 x;
    i32 y;
    i32 w;
    i32 h;
} Rect;

typedef struct {
    u8 red_pos;
    u8 red_mask_size;
    u8 green_pos;
    u8 green_mask_size;
    u8 blue_pos;
    u8 blue_mask_size;
} GuiPixelFormat;

typedef struct {
    void *buffer_ptr;  // Virtual address in userspace where buffer is mapped
    size_t width;
    size_t height;
    size_t pitch;  // Bytes per row
    size_t bpp;    // Bits per pixel (usually 32)
    GuiPixelFormat format;
} GuiBufferInfo;

#endif  // LIBS_LIBC_SRC_INCLUDE_ALKOS_VIDEO_H_
