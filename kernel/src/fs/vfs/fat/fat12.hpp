#ifndef KERNEL_SRC_VFS_FAT_FAT12_HPP_
#define KERNEL_SRC_VFS_FAT_FAT12_HPP_

#include <bit.hpp>
#include <fs/vfs/fat/fat.hpp>
#include <fs/vfs/interface.hpp>
#include <internal/math.hpp>
#include <internal/span.hpp>

namespace vfs
{

/* FAT12 filesystem implementation of the VFS interface
 * https://www.cs.fsu.edu/~cop4610t/assignments/project3/spec/fatspec.pdf
 */
template <VFSIO IO>
class Fat12 : public Fat<Fat12, IO>
{
    using BaseT = Fat<Fat12, IO>;

    public:
    using ClusterNumT = typename BaseT::ClusterNumT;

    // Filesystem identification
    static constexpr Type kFsType        = Type::kFat12;
    static constexpr const char *kFsName = "FAT12";

    // ------------------------------
    // Class creation
    // ------------------------------

    Fat12() = delete;

    explicit Fat12(IO &io)
        : BaseT(io), boot_sector_(internal::get<const BootSector>(io.ReadSector(0)))
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

    ~Fat12() = default;

    friend class Fat<Fat12, IO>;

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

        constexpr u16 kMaxClusterCount = 4085;
        return cluster_count < kMaxClusterCount;
    }

    private:
    struct BootSector {
        typename BaseT::BootSector fat;  // Inherit common fields from Fat

        // FAT12 specific fields
        u8 drive_number;
        RESERVED(1);
        u8 boot_signature;
        u32 volume_id;
        char volume_label[11];  // Padded with spaces
        char filesystem_type[8];
    } PACK;
    static_assert(sizeof(BootSector) == 62, "BootSector size mismatch");

    // FAT12 root directory is at a fixed location, not in a cluster
    ClusterNumT GetRootCluster() const
    {
        // For FAT12, root directory starts at sector after FAT region
        // We return 0 to indicate fixed root directory
        return 0;
    }

    NODISCARD FORCE_INLINE_F ClusterNumT GetFATEntry_(ClusterNumT cluster) const
    {
        ASSERT_LT(
            cluster, BaseT::cluster_count_ + BaseT::kFirstClusterNumber,
            "Cluster number out of range"
        );
        const size_t fat_offset = cluster + (cluster / 2);
        const size_t sector_number =
            BaseT::fat_region_.start + (fat_offset / boot_sector_.fat.bytes_per_sector);
        const size_t sector_offset = fat_offset % boot_sector_.fat.bytes_per_sector;

        // Load 2 sectors if entry spans two sectors
        size_t count =
            (sector_offset == static_cast<size_t>(boot_sector_.fat.bytes_per_sector - 1)) ? 2 : 1;
        auto range = BaseT::io_.ReadRange({sector_number, count});
        if ((cluster % 2) == 0) {  // Even cluster
            return internal::get<ClusterNumT>(range, sector_offset) & kClusterMask;
        } else {
            return internal::get<ClusterNumT>(range, sector_offset) >> 4;
        }
    }

    FORCE_INLINE_F void SetFATEntry_(ClusterNumT cluster, ClusterNumT value)
    {
        ASSERT_LT(
            cluster, BaseT::cluster_count_ + BaseT::kFirstClusterNumber,
            "Cluster number out of range"
        );

        // FAT12 uses 12-bit entries, packed as: entry_n at offset (n * 3 / 2)
        // Even clusters: low 12 bits, Odd clusters: high 12 bits (shifted by 4)
        const size_t fat_offset = cluster + (cluster / 2);
        const size_t sector_number =
            BaseT::fat_region_.start + (fat_offset / boot_sector_.fat.bytes_per_sector);
        const size_t sector_offset = fat_offset % boot_sector_.fat.bytes_per_sector;

        // Load 2 sectors if entry spans two sectors
        size_t count =
            (sector_offset == static_cast<size_t>(boot_sector_.fat.bytes_per_sector - 1)) ? 2 : 1;
        auto range = BaseT::io_.ReadRange({sector_number, count});

        u16 &fat_entry = internal::get<u16>(range, sector_offset);
        if ((cluster % 2) == 0) {  // Even cluster: set low 12 bits
            fat_entry = (fat_entry & 0xF000) | (value & 0x0FFF);
        } else {  // Odd cluster: set high 12 bits
            fat_entry = (fat_entry & 0x000F) | ((value & 0x0FFF) << 4);
        }

        BaseT::io_.WriteRange({sector_number, count}, range);
    }

    static constexpr ClusterNumT kEOC         = 0x0FF8;  // End of cluster chain marker for FAT12
    static constexpr ClusterNumT kClusterMask = kBitMaskRight<ClusterNumT, 12>;

    const BootSector boot_sector_{};
};

template <typename IO>
struct FatTraits<Fat12, IO> {
    using ClusterNumT                      = u16;
    static constexpr bool kHasFixedRootDir = true;
};

}  // namespace vfs

#endif  // KERNEL_SRC_VFS_FAT_FAT12_HPP_
