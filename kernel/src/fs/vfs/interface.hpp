#ifndef KERNEL_SRC_VFS_INTERFACE_HPP_
#define KERNEL_SRC_VFS_INTERFACE_HPP_

#include <types.h>
#include <concepts.hpp>
#include <defines.hpp>
#include <span.hpp>

namespace vfs
{

// Forward declarations
class Path;

namespace io
{

struct SectorRange {
    size_t start;
    size_t count;
};

}  // namespace io

enum class Type { kUnknown, kFat12, kFat16, kFat32 };

template <template <typename> typename T, typename IO>
struct FatTraits {
    static_assert(false, "Invalid FatTraits specialization");
};

template <typename IO>
concept VFSIO =
    requires(IO io, size_t offset, io::SectorRange range, std::span<const byte> data, size_t size) {
        { io.ReadRange(range) } -> std::same_as<std::span<byte>>;
        { io.ReadSector(offset) } -> std::same_as<std::span<byte>>;
        { io.WriteRange(range, data) } -> std::same_as<void>;
        { io.WriteSector(offset, data) } -> std::same_as<void>;
        { io.GetSectorSize() } -> std::same_as<size_t>;
    };

// Forward declare Filesystem struct
struct Filesystem;

/* Should be derived by filesystem driver implementations.
 * This interface defines the basic operations that a filesystem should support.
 * Note: The actual implementation uses Path and Result types, but this concept
 * only checks for the existence of GetFilesystem() as the main API entry point.
 */
template <template <typename> typename T, typename IO>
concept VFSInterface = VFSIO<IO> and requires(T<IO> fs, IO io) {
    T<IO>(io);
    { T<IO>::IsValid(io) } -> std::same_as<bool>;
    { fs.GetFilesystem() } -> std::same_as<Filesystem>;
};

}  // namespace vfs

#endif  // KERNEL_SRC_VFS_INTERFACE_HPP_
