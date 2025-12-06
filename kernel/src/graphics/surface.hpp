#ifndef KERNEL_SRC_GRAPHICS_SURFACE_HPP_
#define KERNEL_SRC_GRAPHICS_SURFACE_HPP_

#include <assert.h>
#include <mem/types.hpp>
#include <types.hpp>

namespace Graphics
{

/**
 * @brief A non-owning view into a linear framebuffer or memory buffer.
 * Assumes 32-bit pixel depth (BPP).
 */
class Surface
{
    public:
    Surface() = default;

    Surface(Mem::VPtr<u32> buffer, u32 width, u32 height, u32 pitch_bytes)
        : buffer_(buffer), width_(width), height_(height), pitch_bytes_(pitch_bytes)
    {
        R_ASSERT_NOT_NULL(buffer_);
        R_ASSERT_NOT_ZERO(width_);
        R_ASSERT_NOT_ZERO(height_);
        R_ASSERT_NOT_ZERO(pitch_bytes_);
    }

    // -------------------------------------------------------------------------
    // Fast Accessors
    // -------------------------------------------------------------------------

    NODISCARD FORCE_INLINE_F Mem::VPtr<u32> GetScanline(u32 y)
    {
        ASSERT_LT(y, height_);
        // Pointer arithmetic on byte level for pitch
        uptr addr = Mem::PtrToUptr(buffer_) + (static_cast<size_t>(y) * pitch_bytes_);
        return Mem::UptrToPtr<u32>(addr);
    }

    NODISCARD FORCE_INLINE_F const Mem::VPtr<u32> GetScanline(u32 y) const
    {
        ASSERT_LT(y, height_);
        uptr addr = Mem::PtrToUptr(buffer_) + (static_cast<size_t>(y) * pitch_bytes_);
        return Mem::UptrToPtr<u32>(addr);
    }

    FORCE_INLINE_F u32 &Pixel(u32 x, u32 y)
    {
        ASSERT_LT(x, width_);
        return GetScanline(y)[x];
    }

    // -------------------------------------------------------------------------
    // Properties
    // -------------------------------------------------------------------------

    NODISCARD FORCE_INLINE_F u32 GetWidth() const { return width_; }
    NODISCARD FORCE_INLINE_F u32 GetHeight() const { return height_; }
    NODISCARD FORCE_INLINE_F u32 GetPitch() const { return pitch_bytes_; }
    NODISCARD FORCE_INLINE_F bool IsValid() const { return buffer_ != nullptr; }

    private:
    Mem::VPtr<u32> buffer_{nullptr};
    u32 width_{0};
    u32 height_{0};
    u32 pitch_bytes_{0};  // Stride in bytes
};

}  // namespace Graphics

#endif  // KERNEL_SRC_GRAPHICS_SURFACE_HPP_
