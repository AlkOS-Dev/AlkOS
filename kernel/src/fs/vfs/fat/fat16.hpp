#ifndef KERNEL_SRC_FS_VFS_FAT_FAT16_HPP_
#define KERNEL_SRC_FS_VFS_FAT_FAT16_HPP_

#include <bit.hpp>
#include <fs/vfs/fat/fat.hpp>
#include <fs/vfs/interface.hpp>
#include <internal/math.hpp>
#include <internal/span.hpp>

namespace vfs
{

/* FAT16 filesystem implementation of the VFS interface
 * https://www.cs.fsu.edu/~cop4610t/assignments/project3/spec/fatspec.pdf
 */
template <VFSIO IO>
class Fat16 : public Fat<Fat16, IO>
{
    using BaseT = Fat<Fat16, IO>;

    public:
    using ClusterNumT = typename BaseT::ClusterNumT;

    // Filesystem identification
    static constexpr Type kFsType        = Type::kFat16;
    static constexpr const char *kFsName = "FAT16";

    // ------------------------------
    // Class creation
    // ------------------------------

    Fat16() = delete;

    explicit Fat16(IO &io) : BaseT(io)
    {
        BaseT::io_.SetSectorSize(boot_sector_.fat.bytes_per_sector);

        const u32 fat_region_start_sector = boot_sector_.fat.reserved_sectors;
        const u32 fat_region_sectors =
            boot_sector_.fat.number_of_fats * boot_sector_.fat.fat_size_word;
        BaseT::fat_region_ = {fat_region_start_sector, fat_region_sectors};

        const u32 root_dir_start   = fat_region_start_sector + fat_region_sectors;
        const u32 root_dir_sectors = internal::DivRoundUp(
            boot_sector_.fat.root_dir_entries * sizeof(typename BaseT::DirectoryEntry),
            boot_sector_.fat.bytes_per_sector
        );
        BaseT::root_dir_region_ = {root_dir_start, root_dir_sectors};

        const u32 data_start_sector =
            boot_sector_.fat.reserved_sectors + fat_region_sectors + root_dir_sectors;
        const u32 data_sectors =
            ((boot_sector_.fat.total_sectors_word != 0) ? boot_sector_.fat.total_sectors_word
                                                        : boot_sector_.fat.total_sectors_dword) -
            data_start_sector;
        BaseT::data_region_ = {data_start_sector, data_sectors};

        BaseT::cluster_count_ = data_sectors / boot_sector_.fat.sectors_per_cluster;
    }

    ~Fat16() = default;

    friend class Fat<Fat16, IO>;

    Filesystem GetFilesystem() { return BaseT::GetFilesystem(); }

    static bool IsValid(IO &io)
    {
        if (!BaseT::IsValid(io)) {
            return false;
        }

        auto boot_sector = internal::get<const BootSector>(io.ReadSector(0));

        const u32 fat_region_sectors =
            boot_sector.fat.number_of_fats * boot_sector.fat.fat_size_word;
        const u32 root_dir_sectors = internal::DivRoundUp(
            boot_sector.fat.root_dir_entries * sizeof(typename BaseT::DirectoryEntry),
            boot_sector.fat.bytes_per_sector
        );
        const u32 data_start_sector =
            boot_sector.fat.reserved_sectors + fat_region_sectors + root_dir_sectors;
        const u32 data_sectors =
            ((boot_sector.fat.total_sectors_word != 0) ? boot_sector.fat.total_sectors_word
                                                       : boot_sector.fat.total_sectors_dword) -
            data_start_sector;
        const u32 cluster_count = data_sectors / boot_sector.fat.sectors_per_cluster;

        constexpr u16 kMinClusterCount = 4085;
        constexpr u16 kMaxClusterCount = 65525;
        return cluster_count >= kMinClusterCount && cluster_count < kMaxClusterCount;
    }

    private:
    struct BootSector {
        typename BaseT::BootSector fat;  // Inherit common fields from Fat

        // FAT16 specific fields
        u8 drive_number;
        RESERVED(1);
        u8 boot_signature;
        u32 volume_id;
        char volume_label[11];  // Padded with spaces
        char filesystem_type[8];
    } PACK;
    static_assert(sizeof(BootSector) == 62, "BootSector size mismatch");

    // FAT16 root directory is at a fixed location, not in a cluster
    ClusterNumT GetRootCluster() const
    {
        // For FAT16, root directory starts at sector after FAT region
        // We return 0 to indicate fixed root directory
        return 0;
    }

    static constexpr u16 kEOC                 = 0xFFF8;  // End of cluster chain marker for FAT16
    static constexpr ClusterNumT kClusterMask = kFullMask<ClusterNumT>;
    static constexpr ClusterNumT kCleanShutdownMarkerBit = sizeof(ClusterNumT) * 8 - 1;
    static constexpr ClusterNumT kHardwareErrorMarkerBit = sizeof(ClusterNumT) * 8 - 2;

    BootSector boot_sector_;
};

template <typename IO>
struct FatTraits<Fat16, IO> {
    using ClusterNumT                      = u16;
    static constexpr bool kHasFixedRootDir = true;
};

}  // namespace vfs

#endif  // KERNEL_SRC_FS_VFS_FAT_FAT16_HPP_
