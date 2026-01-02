#ifndef LIBS_LIBC_SRC_INCLUDE_SYS_VIDEO_H_
#define LIBS_LIBC_SRC_INCLUDE_SYS_VIDEO_H_

#include <defines.h>
#include <stddef.h>
#include <stdint.h>

BEGIN_DECL_C

struct GuiPixelFormat {
    uint8_t red_pos;
    uint8_t red_mask_size;
    uint8_t green_pos;
    uint8_t green_mask_size;
    uint8_t blue_pos;
    uint8_t blue_mask_size;
};

struct GuiBufferInfo {
    void *buffer_ptr;  // Virtual address in userspace where buffer is mapped
    size_t width;
    size_t height;
    size_t pitch;  // Bytes per row
    size_t bpp;    // Bits per pixel (usually 32)
    struct GuiPixelFormat format;
};

END_DECL_C

#endif  // LIBS_LIBC_SRC_INCLUDE_SYS_VIDEO_H_
