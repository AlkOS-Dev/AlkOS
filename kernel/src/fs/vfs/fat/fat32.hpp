#ifndef KERNEL_SRC_FS_VFS_FAT_FAT32_HPP_
#define KERNEL_SRC_FS_VFS_FAT_FAT32_HPP_

#include <bit.hpp>
#include <fs/vfs/fat/fat.hpp>
#include <fs/vfs/interface.hpp>
#include <internal/math.hpp>
#include <internal/span.hpp>

namespace vfs
{

/* FAT32 filesystem implementation of the VFS interface
 * https://www.cs.fsu.edu/~cop4610t/assignments/project3/spec/fatspec.pdf
 */
template <VFSIO IO>
class Fat32 : public Fat<Fat32, IO>
{
    using BaseT = Fat<Fat32, IO>;

    public:
    using ClusterNumT = typename BaseT::ClusterNumT;

    // Filesystem identification
    static constexpr Type kFsType        = Type::kFat32;
    static constexpr const char *kFsName = "FAT32";

    // ------------------------------
    // Class creation
    // ------------------------------

    Fat32() = delete;

    explicit Fat32(IO &io) : BaseT(io)
    {
        BaseT::io_.SetSectorSize(boot_sector_.fat.bytes_per_sector);

        const u32 fat_region_start_sector = boot_sector_.fat.reserved_sectors;
        const u32 fat_region_sectors =
            boot_sector_.fat.number_of_fats * ((boot_sector_.fat.fat_size_word != 0)
                                                   ? boot_sector_.fat.fat_size_word
                                                   : boot_sector_.fat_size_dword);
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

    ~Fat32() = default;

    friend class Fat<Fat32, IO>;

    Filesystem GetFilesystem() { return BaseT::GetFilesystem(); }

    static bool IsValid(IO &io)
    {
        if (!BaseT::IsValid(io)) {
            return false;
        }

        auto boot_sector = internal::get<const BootSector>(io.ReadSector(0));

        if (boot_sector.fat.fat_size_word != 0 || boot_sector.fat.total_sectors_word != 0) {
            return false;
        }

        const u32 fat_region_sectors = boot_sector.fat.number_of_fats * boot_sector.fat_size_dword;
        const u32 root_dir_sectors   = internal::DivRoundUp(
            boot_sector.fat.root_dir_entries * sizeof(typename BaseT::DirectoryEntry),
            boot_sector.fat.bytes_per_sector
        );
        const u32 data_start_sector =
            boot_sector.fat.reserved_sectors + fat_region_sectors + root_dir_sectors;
        const u32 data_sectors  = boot_sector.fat.total_sectors_dword - data_start_sector;
        const u32 cluster_count = data_sectors / boot_sector.fat.sectors_per_cluster;

        constexpr u16 kMinClusterCount = 65525;
        return cluster_count >= kMinClusterCount;
    }

    private:
    struct BootSector {
        typename BaseT::BootSector fat;  // Inherit common fields from Fat

        // FAT32 specific fields
        u32 fat_size_dword;
        u16 extended_flags;
        u16 filesystem_version;
        u32 root_cluster;
        u16 filesystem_info_sector;
        u16 backup_boot_sector;
        RESERVED(12);
        u8 drive_number;
        RESERVED(1);
        u8 boot_signature;
        u32 volume_id;
        char volume_label[11];  // Padded with spaces
        char filesystem_type[8];
    } PACK;
    static_assert(sizeof(BootSector) == 90, "BootSector size mismatch");

    struct FSInfo {
        u32 lead_signature;  // 0x41615252
        RESERVED(480);
        u32 structure_signature;  // 0x61417272
        u32 free_cluster_count;
        u32 next_free_cluster;  // if 0xFFFFFFFF start searching from cluster 2
        RESERVED(12);
        u32 trail_signature;  // 0xAA550000
    };
    static_assert(sizeof(FSInfo) == 512, "FSInfo size mismatch");

    ClusterNumT GetRootCluster() const { return boot_sector_.root_cluster; }

    // FAT32 specific constants
    static constexpr u32 kFSInfoLeadSig      = 0x41615252;
    static constexpr u32 kFSInfoStructureSig = 0x61417272;
    static constexpr u32 kFSInfoTrailSig     = 0xAA550000;

    // FAT32 specific cluster markers
    static constexpr ClusterNumT kEOC =
        kBitMask<ClusterNumT, 3, 25>;  // End of cluster marker for FAT32
    static constexpr ClusterNumT kClusterMask = kBitMaskRight<ClusterNumT, 28>;
    static constexpr ClusterNumT kCleanShutdownMarker =
        kSingleBit<ClusterNumT, sizeof(ClusterNumT) * 8 - 5>;
    static constexpr ClusterNumT kHardwareErrorMarker =
        kSingleBit<ClusterNumT, sizeof(ClusterNumT) * 8 - 6>;

    // ------------------------------
    // Data Members
    // ------------------------------

    BootSector boot_sector_;
    FSInfo fs_info_;
};

template <typename IO>
struct FatTraits<Fat32, IO> {
    using ClusterNumT                      = u32;
    static constexpr bool kHasFixedRootDir = false;
};

}  // namespace vfs

#endif  // KERNEL_SRC_FS_VFS_FAT_FAT32_HPP_
