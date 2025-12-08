#ifndef KERNEL_SRC_GRAPHICS_DONUT_DONUT_HPP_
#define KERNEL_SRC_GRAPHICS_DONUT_DONUT_HPP_

#include <algorithm.hpp>
#include <memory.hpp>
#include <types.hpp>

#include "graphics/donut/math.hpp"

#include "graphics/color.hpp"
#include "graphics/native_pixel.hpp"
#include "graphics/painter.hpp"
#include "graphics/surface.hpp"
#include "mem/heap.hpp"

class SpinningDonut
{
    public:
    void Init(u32 width, u32 height)
    {
        width_  = width;
        height_ = height;

        size_t zbuf_size = width * height * sizeof(f32);
        auto alloc       = Mem::KMalloc(zbuf_size);
        if (alloc) {
            z_buffer_ = static_cast<f32 *>(*alloc);
        }
    }

    void Render(Graphics::Surface &screen, Graphics::Painter &painter)
    {
        if (z_buffer_ == nullptr) {
            return;
        }

        // we treat 1/z as depth (larger is closer)
        memset(z_buffer_, 0, width_ * height_ * sizeof(f32));

        painter.Clear(Graphics::Color::Black());

        // Precompute sines and cosines of rotation angles
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

                // 3D coordinates calculation (The donut math)
                f32 circleX = R2 + (R1 * ct);
                f32 circleY = R1 * st;

                f32 x = (circleX * (cp * cB + sp * sA * sB)) - (circleY * cA * sB);
                f32 y = (circleX * (cp * sB - sp * sA * cB)) + (circleY * cA * cB);
                f32 z = K2 + (circleX * sp * cA) + (circleY * sA);

                // One over Z (Depth)
                f32 ooz = 1.0F / z;

                // x and y projection (Screen coordinates)
                int xp = (int)((width_ / 2) + (K1 * ooz * x));
                int yp = (int)((height_ / 2) - (K1 * ooz * y));

                // Luminance (Lighting)
                // L ranges from -sqrt(2) to +sqrt(2).
                f32 L =
                    (cp * ct * sB) - (cA * ct * sp) - (sA * st) + (cB * (cA * st - ct * sA * sp));

                if (L > 0 && xp >= 0 && xp < (int)width_ && yp >= 0 && yp < (int)height_) {
                    int idx = xp + (yp * width_);

                    if (ooz > z_buffer_[idx]) {
                        z_buffer_[idx] = ooz;

                        // Calculate color based on Luminance (L)
                        int luminance = (int)(L * 8.0F);
                        luminance     = std::max(luminance, 0);

                        u8 r = (u8)(255 * (luminance / 12.0F));
                        u8 g = (u8)(150 * (luminance / 12.0F));
                        u8 b = (u8)(50 * (luminance / 12.0F));

                        painter.SetColor({.r = r, .g = g, .b = b});
                        painter.DrawPixel({.x = xp, .y = yp});
                    }
                }
            }
        }

        // Increment rotation
        A += 0.04f;
        B += 0.02f;
    }

    private:
    f32 A = 0;
    f32 B = 0;

    // Donut Geometry Constants
    const f32 R1 = 1.0f;  // Tube radius
    const f32 R2 = 2.0f;  // Distance from center
    const f32 K2 = 5.0f;  // Distance from camera

    // Scale factor (Zoom). Depends on screen size usually.
    // 150-300 is good for 640x480 to 1024x768
    const f32 K1 = 350.0f;

    u32 width_     = 0;
    u32 height_    = 0;
    f32 *z_buffer_ = nullptr;
};

#endif /* KERNEL_SRC_GRAPHICS_DONUT_DONUT_HPP_ */
