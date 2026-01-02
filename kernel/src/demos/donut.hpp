#ifndef KERNEL_SRC_DEMOS_DONUT_HPP_
#define KERNEL_SRC_DEMOS_DONUT_HPP_

#include <stdio.h>
#include <types.h>
#include <algorithm.hpp>
#include <memory.hpp>
#include <span.hpp>
#include <string.hpp>

#include "graphics/color.hpp"
#include "graphics/font/psf2_font.hpp"
#include "graphics/fonts/drdos8x8.hpp"
#include "graphics/native_pixel.hpp"
#include "graphics/painter.hpp"
#include "graphics/surface.hpp"
#include "mem/heap.hpp"
#include "modules/video.hpp"
#include "trace_framework.hpp"

namespace Demos
{

// --------------------------------------------------------------------------------
// Minimal Math for donut Implementation
// --------------------------------------------------------------------------------

namespace Math
{
// Simple Taylor series approximation for sin/cos to avoid libm dependency
// or massive lookup tables. Precision is "good enough" for a donut.
constexpr f32 PI = 3.14159265359f;

f32 sin(f32 x)
{
    // Normalize to -PI to PI
    while (x < -PI) {
        x += 2 * PI;
    }
    while (x > PI) {
        x -= 2 * PI;
    }

    f32 res  = 0;
    f32 term = x;
    f32 k    = 1;
    for (i32 i = 0; i < 5; ++i) {  // 5 iterations is plenty for visual
        res += term;
        term *= -1 * x * x / ((k + 1) * (k + 2));
        k += 2;
    }
    return res;
}

f32 cos(f32 x) { return sin(x + (PI / 2.0F)); }
}  // namespace Math

class SpinningDonut
{
    public:
    void Init(u32 width, u32 height, const Graphics::PixelFormat &format)
    {
        width_  = width;
        height_ = height;

        size_t zbuf_size = width * height * sizeof(f32);
        auto alloc       = Mem::KMalloc(zbuf_size);
        if (alloc) {
            z_buffer_ = static_cast<f32 *>(*alloc);
        }

        // Pre-calculate luminance palette to avoid color packing in the render loop.
        for (i32 i = 0; i < 13; ++i) {
            f32 intensity = i / 12.0f;
            u8 r          = static_cast<u8>(255 * intensity);
            u8 g          = static_cast<u8>(150 * intensity);
            u8 b          = static_cast<u8>(50 * intensity);

            luminance_palette_[i] = Graphics::NativePixel::FromColor({r, g, b}, format);
        }
    }

    void Render(Graphics::Painter &painter)
    {
        if (z_buffer_ == nullptr) {
            return;
        }

        // Reset Z-buffer.
        memset(z_buffer_, 0, width_ * height_ * sizeof(f32));

        painter.Clear(Graphics::Color::Black());

        // Get unsafe pointers for direct access
        Graphics::Surface &screen = painter.GetTarget();
        u8 *raw_byte_buffer       = reinterpret_cast<u8 *>(screen.GetRawBuffer());
        u32 pitch                 = screen.GetPitch();

        // Precompute sines and cosines
        f32 cA = Math::cos(A);
        f32 sA = Math::sin(A);
        f32 cB = Math::cos(B);
        f32 sB = Math::sin(B);

        // Theta (cross section of torus)
        for (f32 theta = 0; theta < 2 * Math::PI; theta += 0.07F) {
            f32 ct = Math::cos(theta);
            f32 st = Math::sin(theta);

            // Phi (center of revolution)
            for (f32 phi = 0; phi < 2 * Math::PI; phi += 0.02F) {
                f32 cp = Math::cos(phi);
                f32 sp = Math::sin(phi);

                // 3D coordinates calculation
                f32 circleX = R2 + (R1 * ct);
                f32 circleY = R1 * st;

                f32 x = (circleX * (cp * cB + sp * sA * sB)) - (circleY * cA * sB);
                f32 y = (circleX * (cp * sB - sp * sA * cB)) + (circleY * cA * cB);
                f32 z = K2 + (circleX * sp * cA) + (circleY * sA);

                // Inverse Z (Depth)
                f32 ooz = 1.0F / z;

                // Screen projection
                i32 xp = (i32)((width_ / 2) + (K1 * ooz * x));
                i32 yp = (i32)((height_ / 2) - (K1 * ooz * y));

                // Luminance
                f32 L =
                    (cp * ct * sB) - (cA * ct * sp) - (sA * st) + (cB * (cA * st - ct * sA * sp));

                if (L > 0 && xp >= 0 && xp < (i32)width_ && yp >= 0 && yp < (i32)height_) {
                    i32 idx = xp + (yp * width_);

                    if (ooz > z_buffer_[idx]) {
                        z_buffer_[idx] = ooz;

                        i32 luminance_idx = (i32)(L * 8.0F);
                        if (luminance_idx > 12)
                            luminance_idx = 12;
                        else if (luminance_idx < 0)
                            luminance_idx = 0;

                        // Direct memory write: Base + (Y * Pitch) + (X * BytesPerPixel)
                        u32 *pixel_addr =
                            reinterpret_cast<u32 *>(raw_byte_buffer + (yp * pitch) + (xp * 4));
                        *pixel_addr = luminance_palette_[luminance_idx].value;
                    }
                }
            }
        }

        A += 0.04f;
        B += 0.02f;
    }

    private:
    f32 A        = 0;
    f32 B        = 0;
    const f32 R1 = 1.0f;    // Tube radius
    const f32 R2 = 2.0f;    // Distance from center
    const f32 K2 = 5.0f;    // Distance from camera
    const f32 K1 = 350.0f;  // Zoom/Scale

    u32 width_     = 0;
    u32 height_    = 0;
    f32 *z_buffer_ = nullptr;

    Graphics::NativePixel luminance_palette_[13];
};

void RunDonutDemo()
{
    auto &video  = VideoModule::Get();
    auto &screen = video.GetScreen();
    auto fmt     = video.GetFormat();

    Graphics::Painter p(screen, fmt);
    Graphics::Psf2Font system_font(std::span<const u8>(drdos8x8_psfu, drdos8x8_psfu_len));

    if (!system_font.IsValid()) {
        TRACE_WARN_VIDEO("System font magic invalid! Rendering might be corrupted.");
    }

    SpinningDonut donut;
    donut.Init(screen.GetWidth(), screen.GetHeight(), fmt);

    u64 frame_count = 0;
    char text_buffer[100];
    while (true) {
        donut.Render(p);

        p.SetColor(Graphics::Color::White());
        p.DrawString(
            {.x = 10, .y = 10, .text = "AlkOS Kernel - Graphics Module ON", .scale = 2}, system_font
        );
        i32 ret = snprintf(text_buffer, 100, "Frame %lli", frame_count);
        (void)ret;
        p.DrawString(
            {.x     = (static_cast<i32>(
                 screen.GetWidth() - system_font.MeasureString(text_buffer).width * 2
             )),
             .y     = 10,
             .text  = text_buffer,
             .scale = 2},
            system_font
        );
        video.Flush();

        for (volatile i32 i = 0; i < 10000000; i++) {
            ;
        }

        frame_count++;
    }
}

}  // namespace Demos

#endif  // KERNEL_SRC_DEMOS_DONUT_HPP_
