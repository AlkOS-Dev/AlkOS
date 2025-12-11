#ifndef KERNEL_SRC_VFS_IO_IN_MEMORY_HPP_
#define KERNEL_SRC_VFS_IO_IN_MEMORY_HPP_

#include <string.h>
#include <mem/heap.hpp>
#include <span.hpp>
#include <todo.hpp>
#include <vfs/interface.hpp>

namespace vfs::io
{

class InMemory
{
    static constexpr size_t kMaxTempBufSize = 64 * 1024;  // 64 KB

    public:
    // ------------------------------
    // Class creation
    // ------------------------------
    InMemory() = delete;
    explicit InMemory(void *address, size_t sector_size = 512)
        : address_(static_cast<byte *>(address)), sector_size_(sector_size), buffer_(nullptr)
    {
        auto result = Mem::KMalloc(kMaxTempBufSize);
        R_ASSERT_TRUE(result.has_value(), "Failed to allocate InMemory buffer");
        buffer_ = static_cast<byte *>(result.value());
    }

    ~InMemory()
    {
        if (buffer_) {
            Mem::KFree(buffer_);
            buffer_ = nullptr;
        }
    }

    InMemory(const InMemory &)            = delete;
    InMemory &operator=(const InMemory &) = delete;

    InMemory(InMemory &&other) noexcept
        : address_(other.address_), sector_size_(other.sector_size_), buffer_(other.buffer_)
    {
        other.buffer_  = nullptr;
        other.address_ = nullptr;
    }

    InMemory &operator=(InMemory &&other) noexcept
    {
        if (this != &other) {
            if (buffer_) {
                Mem::KFree(buffer_);
            }
            address_       = other.address_;
            sector_size_   = other.sector_size_;
            buffer_        = other.buffer_;
            other.buffer_  = nullptr;
            other.address_ = nullptr;
        }
        return *this;
    }

    std::span<byte> ReadRange(SectorRange range)
    {
        size_t total_size = range.count * sector_size_;
        R_ASSERT_LE(
            total_size, kMaxTempBufSize, "Requested range exceeds maximum temporary buffer size"
        );

        memcpy(buffer_, address_ + range.start * sector_size_, total_size);
        return std::span<byte>(buffer_, total_size);
    }

    std::span<byte> ReadSector(size_t offset) { return ReadRange({offset, 1}); }

    void WriteRange(SectorRange range, std::span<const byte> data)
    {
        ASSERT_EQ(
            data.size(), range.count * sector_size_,
            "Data size does not match the expected size for writing"
        );

        memcpy(address_ + range.start * sector_size_, data.data(), range.count * sector_size_);
    }

    FORCE_INLINE_F void WriteSector(size_t offset, std::span<const byte> data)
    {
        WriteRange({offset, 1}, data);
    }

    FORCE_INLINE_F size_t GetSectorSize() const { return sector_size_; }

    FORCE_INLINE_F void SetSectorSize(size_t sector_size) { sector_size_ = sector_size; }

    private:
    byte *address_;
    size_t sector_size_;
    byte *buffer_;
};

static_assert(VFSIO<InMemory>, "InMemory does not implement VFSIO");

}  // namespace vfs::io

#endif  // KERNEL_SRC_VFS_IO_IN_MEMORY_HPP_
