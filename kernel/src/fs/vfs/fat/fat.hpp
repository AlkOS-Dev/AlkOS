#ifndef KERNEL_SRC_VFS_FAT_FAT_HPP_
#define KERNEL_SRC_VFS_FAT_FAT_HPP_

#include <ctype.h>
#include <string.h>
#include <fs/vfs/error.hpp>
#include <fs/vfs/interface.hpp>
#include <fs/vfs/path.hpp>
#include <internal/span.hpp>
#include <span.hpp>
#include <string.hpp>
#include <template_lib.hpp>
#include <trace_framework.hpp>
#include "macros.hpp"

namespace vfs
{

template <template <typename> typename T, typename IO>
class Fat
{
    using ImplT  = T<IO>;
    using Traits = FatTraits<T, IO>;

    protected:
    using ClusterNumT                      = typename Traits::ClusterNumT;
    static constexpr bool kHasFixedRootDir = Traits::kHasFixedRootDir;

    // ------------------------------
    // Construction
    // ------------------------------

    Fat() = delete;

    explicit Fat(IO &io) : io_(io)
    {
        if constexpr (requires { ImplT::kCleanShutdownMarker; }) {
            if (!IsBitEnabled<ImplT::kCleanShutdownMarker>(GetFATEntry_(1))) {
                TRACE_WARN_VFS(
                    "Filesystem has not been shut down properly, some data may be lost!"
                );
            }
        }
    }

    ~Fat() = default;

    // ------------------------------
    // FAT Structures
    // ------------------------------

    struct BootSector {
        static constexpr u16 kSignature = 0xAA55;

        // FAT12/FAT16/FAT32 common fields
        byte jump_instruction[3];
        char oem_name[8];
        u16 bytes_per_sector;
        u8 sectors_per_cluster;
        u16 reserved_sectors;
        u8 number_of_fats;
        u16 root_dir_entries;
        u16 total_sectors_word;  // 0 if FAT32 then total_sectors_dword is used
        u8 media_descriptor;     // 0xF8 for fixed disk, 0xF0 for removable disk
        u16 fat_size_word;       // 0 if FAT32
        u16 sectors_per_track;
        u16 number_of_heads;
        u32 hidden_sectors;
        u32 total_sectors_dword;
    } PACK;
    static_assert(sizeof(BootSector) == 36, "BootSector size mismatch");

    struct LongDirectoryEntry {
        static constexpr u8 kLastLongEntryMask = 0x40;

        NODISCARD FORCE_INLINE_F bool IsLast() const { return (order & kLastLongEntryMask) != 0; }

        u8 order;               // Order of the entry in the long name sequence
        char name1[10];         // First 5 characters of the long name
        u8 attributes;          // Must be LongName (0x0F)
        u8 type;                // Must be 0x00
        u8 checksum;            // Checksum for the short name
        char name2[12];         // Next 6 characters of the long name
        u16 first_cluster_low;  // Must be zero
        char name3[4];          // Last 2 characters of the long name
    } PACK;
    static_assert(sizeof(LongDirectoryEntry) == 32, "LongDirectoryEntry size mismatch");

    struct DirectoryEntry {
        static constexpr u8 kLongNameMask = 0x3F;
        static constexpr u8 kDeletedEntry = 0xE5;
        static constexpr u8 kEOD          = 0x00;

        enum class Attributes : u8 {
            None        = 0x00,
            ReadOnly    = 0x01,
            Hidden      = 0x02,
            System      = 0x04,
            VolumeLabel = 0x08,
            Directory   = 0x10,
            Archive     = 0x20,
            LongName    = 0x0F,  // ReadOnly | Hidden | System | VolumeLabel
        };

        friend constexpr Attributes operator|(Attributes a, Attributes b)
        {
            return static_cast<Attributes>(static_cast<u8>(a) | static_cast<u8>(b));
        }

        friend constexpr Attributes operator&(Attributes a, Attributes b)
        {
            return static_cast<Attributes>(static_cast<u8>(a) & static_cast<u8>(b));
        }

        friend constexpr Attributes operator&(Attributes a, u8 b)
        {
            return static_cast<Attributes>(static_cast<u8>(a) & b);
        }

        friend constexpr bool operator==(Attributes a, Attributes b)
        {
            return static_cast<u8>(a) == static_cast<u8>(b);
        }

        NODISCARD FORCE_INLINE_F bool IsLongNameSubComponent() const
        {
            return ((attributes & kLongNameMask) == Attributes::LongName) && !IsFree();
        }

        NODISCARD FORCE_INLINE_F LongDirectoryEntry *TryGetLongNameSubComponent()
        {
            return IsLongNameSubComponent() ? reinterpret_cast<LongDirectoryEntry *>(this)
                                            : nullptr;
        }

        NODISCARD FORCE_INLINE_F u8 GetCheckSum() const
        {
            u8 sum   = 0;
            auto ptr = reinterpret_cast<const unsigned char *>(filename);
            for (size_t i = kMaxNameLength; i != 0; --i) {
                sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *ptr++;
            }
            return sum;
        }

        NODISCARD FORCE_INLINE_F bool IsDirectory() const
        {
            return (attributes & (Attributes::Directory | Attributes::VolumeLabel)) ==
                   Attributes::Directory;
        }

        NODISCARD FORCE_INLINE_F bool IsFile() const
        {
            return (attributes & (Attributes::Directory | Attributes::VolumeLabel)) ==
                   Attributes::None;
        }

        NODISCARD FORCE_INLINE_F bool IsVolumeLabel() const
        {
            return (attributes & (Attributes::Directory | Attributes::VolumeLabel)) ==
                   Attributes::VolumeLabel;
        }

        NODISCARD FORCE_INLINE_F bool IsValid() const
        {
            return IsFile() || IsDirectory() || IsVolumeLabel();
        }

        NODISCARD FORCE_INLINE_F bool IsRootDirectory() const
        {
            return IsDirectory() && first_cluster_low == 0 && first_cluster_high == 0;
        }

        NODISCARD FORCE_INLINE_F bool IsFree() const
        {
            return reinterpret_cast<const byte &>(*this) == kDeletedEntry;
        }
        NODISCARD FORCE_INLINE_F bool IsFurtherEntriesFree() const
        {
            return reinterpret_cast<const byte &>(*this) == kEOD;
        }

        char filename[11];  // Padded with spaces
        Attributes attributes;
        RESERVED(1);
        u8 creation_time_tenth;  // Time in tenths of seconds, valid range 0-199
        u16 creation_time;       // Time in 2-second intervals
        u16 creation_date;       // Date relative to MS-DOS epoch of 01/01/1980.
        u16 last_access_date;
        u16 first_cluster_high;  // High 16 bits of the first cluster number
        u16 last_write_time;     // Must be supported
        u16 last_write_date;     // Must be supported
        u16 first_cluster_low;   // Low 16 bits of the first cluster number
        u32 file_size;           // Size of the file in bytes
    } PACK;
    static_assert(sizeof(DirectoryEntry) == 32, "DirectoryEntry size mismatch");

    struct PathLookupResult {
        DirectoryEntry entry;
        ClusterNumT parent_cluster;
        size_t entry_offset;  // Offset within the parent directory cluster
        bool found;
    };

    // ------------------------------
    // Constants
    // ------------------------------

    static constexpr u16 kMaxDirectoryEntries   = static_cast<u16>(-1);
    static constexpr size_t kFirstClusterNumber = 2;
    static constexpr size_t kMaxNameLength      = 11;
    static constexpr size_t kMaxClusterSize     = 32768;
    static constexpr auto kDot    = template_lib::fill_array_v<char, kMaxNameLength, ' ', '.'>;
    static constexpr auto kDotDot = template_lib::fill_array_v<char, kMaxNameLength, ' ', '.', '.'>;

    // ------------------------------
    // Validation
    // ------------------------------

    NODISCARD static bool IsValid(IO &io)
    {
        auto sector_data = io.ReadSector(0);

        if (!ValidateBootSignature_(sector_data)) {
            return false;
        }

        auto boot_sector = internal::get<const BootSector>(sector_data);

        if (!ValidateJumpInstruction_(boot_sector)) {
            return false;
        }

        if (!ValidateSectorAndClusterSize_(boot_sector)) {
            return false;
        }

        if (!ValidateReservedSectors_(boot_sector)) {
            return false;
        }

        if (!ValidateFatCount_(boot_sector)) {
            return false;
        }

        if (!ValidateMediaDescriptor_(boot_sector)) {
            return false;
        }

        if (!ValidateFatMediaDescriptor_(io, boot_sector)) {
            return false;
        }

        return true;
    }

    // ------------------------------
    // VFS Filesystem Interface
    // ------------------------------

    NODISCARD Filesystem GetFilesystem()
    {
        return Filesystem(
            this,
            Filesystem::Operations{
                .create_file      = &Fat::CreateFileCallback_,
                .read_file        = &Fat::ReadFileCallback_,
                .write_file       = &Fat::WriteFileCallback_,
                .delete_file      = &Fat::DeleteFileCallback_,
                .file_exists      = &Fat::FileExistsCallback_,
                .get_file_size    = &Fat::GetFileSizeCallback_,
                .create_directory = &Fat::CreateDirectoryCallback_,
                .remove_directory = &Fat::RemoveDirectoryCallback_,
                .list_directory   = &Fat::ListDirectoryCallback_,
                .directory_exists = &Fat::DirectoryExistsCallback_,
                .exists           = &Fat::ExistsCallback_,
                .move             = &Fat::MoveCallback_,
            },
            Filesystem::Info{
                .type = ImplT::kFsType,
                .name = ImplT::kFsName,
            }
        );
    }

    // ------------------------------
    // File Operations
    // ------------------------------

    NODISCARD Result<> CreateFile(const Path &path)
    {
        RET_UNEXPECTED_IF(path.IsEmpty() || !path.HasComponents(), VfsError::kInvalidPath);

        char formatted_name[kMaxNameLength];
        RET_UNEXPECTED_IF(
            !FormatFilename_(path.GetFilename(), formatted_name), VfsError::kInvalidName
        );

        RET_UNEXPECTED_IF(LookupPath_(path).found, VfsError::kAlreadyExists);

        auto parent_cluster = GetParentCluster_(path);
        RET_UNEXPECTED_IF_ERR(parent_cluster);

        return CreateEntry_(
            parent_cluster.value(), formatted_name, DirectoryEntry::Attributes::Archive, 0
        );
    }

    NODISCARD Result<size_t> ReadFile(const Path &path, void *buffer, size_t size, size_t offset)
    {
        RET_UNEXPECTED_IF(!buffer || size == 0, VfsError::kInvalidArgument);

        auto lookup = LookupPath_(path);
        RET_UNEXPECTED_IF(!lookup.found, VfsError::kFileNotFound);

        RET_UNEXPECTED_IF(!lookup.entry.IsFile(), VfsError::kNotAFile);

        if (offset >= lookup.entry.file_size) {
            return 0;
        }

        return ReadFileData_(lookup.entry, buffer, size, offset);
    }

    NODISCARD Result<size_t> WriteFile(
        const Path &path, const void *buffer, size_t size, size_t offset
    )
    {
        RET_UNEXPECTED_IF(!buffer || size == 0, VfsError::kInvalidArgument);

        auto lookup = LookupPath_(path);
        RET_UNEXPECTED_IF(!lookup.found, VfsError::kFileNotFound);

        RET_UNEXPECTED_IF(!lookup.entry.IsFile(), VfsError::kNotAFile);

        return WriteFileData_(lookup, buffer, size, offset);
    }

    NODISCARD Result<> DeleteFile(const Path &path)
    {
        auto lookup = LookupPath_(path);
        RET_UNEXPECTED_IF(!lookup.found, VfsError::kFileNotFound);

        RET_UNEXPECTED_IF(!lookup.entry.IsFile(), VfsError::kNotAFile);

        return DeleteEntry_(lookup);
    }

    NODISCARD Result<bool> FileExists(const Path &path)
    {
        auto lookup = LookupPath_(path);
        return lookup.found && lookup.entry.IsFile();
    }

    NODISCARD Result<size_t> GetFileSize(const Path &path)
    {
        auto lookup = LookupPath_(path);
        RET_UNEXPECTED_IF(!lookup.found, VfsError::kFileNotFound);
        RET_UNEXPECTED_IF(!lookup.entry.IsFile(), VfsError::kNotAFile);
        return static_cast<size_t>(lookup.entry.file_size);
    }

    // ------------------------------
    // Directory Operations
    // ------------------------------

    NODISCARD Result<> CreateDirectory(const Path &path)
    {
        RET_UNEXPECTED_IF(path.IsEmpty() || !path.HasComponents(), VfsError::kInvalidPath);

        char formatted_name[kMaxNameLength];
        RET_UNEXPECTED_IF(
            !FormatFilename_(path.GetFilename(), formatted_name), VfsError::kInvalidName
        );

        RET_UNEXPECTED_IF(LookupPath_(path).found, VfsError::kAlreadyExists);

        auto parent_cluster = GetParentCluster_(path);
        RET_UNEXPECTED_IF_ERR(parent_cluster);

        return CreateDirectoryWithCluster_(parent_cluster.value(), formatted_name);
    }

    NODISCARD Result<> RemoveDirectory(const Path &path)
    {
        auto lookup = LookupPath_(path);
        RET_UNEXPECTED_IF(!lookup.found, VfsError::kDirectoryNotFound);

        RET_UNEXPECTED_IF(!lookup.entry.IsDirectory(), VfsError::kNotADirectory);

        RET_UNEXPECTED_IF(!IsDirectoryEmpty_(lookup.entry), VfsError::kNotEmpty);

        return DeleteEntry_(lookup);
    }

    void ListDirectory(const Path &path, ListDirCallback callback, void *user_ctx)
    {
        auto lookup = GetDirectoryLookup_(path);
        if (!lookup.found || !lookup.entry.IsDirectory()) {
            return;
        }

        ScanDirectoryTable_(lookup.entry, [callback, user_ctx](const DirectoryEntry &entry) {
            if (entry.IsValid() && !entry.IsVolumeLabel()) {
                char name[13];
                FormatShortName_(entry.filename, name);
                callback(user_ctx, name, entry.IsDirectory());
            }
        });
    }

    NODISCARD Result<bool> DirectoryExists(const Path &path)
    {
        if (path.IsRoot()) {
            return true;
        }
        auto lookup = LookupPath_(path);
        return lookup.found && lookup.entry.IsDirectory();
    }

    NODISCARD Result<bool> Exists(const Path &path)
    {
        if (path.IsRoot()) {
            return true;
        }
        return LookupPath_(path).found;
    }

    // ------------------------------
    // Move Operation
    // ------------------------------

    NODISCARD Result<> Move(const Path &old_path, const Path &new_path)
    {
        auto old_lookup = LookupPath_(old_path);
        RET_UNEXPECTED_IF(!old_lookup.found, VfsError::kFileNotFound);

        RET_UNEXPECTED_IF(LookupPath_(new_path).found, VfsError::kAlreadyExists);

        char new_formatted[kMaxNameLength];
        RET_UNEXPECTED_IF(
            !FormatFilename_(new_path.GetFilename(), new_formatted), VfsError::kInvalidName
        );

        auto new_parent_cluster = GetParentCluster_(new_path);
        RET_UNEXPECTED_IF_ERR(new_parent_cluster);

        return MoveEntry_(old_lookup, new_formatted, new_parent_cluster.value());
    }

    // ------------------------------
    // Protected Cluster Operations
    // ------------------------------

    NODISCARD FAST_CALL u32 GetFirstCluster_(const DirectoryEntry &entry)
    {
        return (static_cast<u32>(entry.first_cluster_high) << 16) | entry.first_cluster_low;
    }

    NODISCARD FORCE_INLINE_F u32 GetClusterSector_(ClusterNumT cluster) const
    {
        return data_region_.start +
               (cluster - kFirstClusterNumber) * GetBootSector_().sectors_per_cluster;
    }

    NODISCARD FORCE_INLINE_F ClusterNumT GetFATEntry_(ClusterNumT cluster)
    {
        auto boot_sector     = GetBootSector_();
        size_t fat_offset    = cluster * sizeof(ClusterNumT);
        size_t sector_number = fat_region_.start + (fat_offset / boot_sector.bytes_per_sector);
        size_t sector_offset = fat_offset % boot_sector.bytes_per_sector;
        return internal::get<ClusterNumT>(io_.ReadSector(sector_number), sector_offset) &
               ImplT::kClusterMask;
    }

    FORCE_INLINE_F void SetFATEntry_(ClusterNumT cluster, ClusterNumT value)
    {
        auto boot_sector     = GetBootSector_();
        size_t fat_offset    = cluster * sizeof(ClusterNumT);
        size_t sector_number = fat_region_.start + (fat_offset / boot_sector.bytes_per_sector);
        size_t sector_offset = fat_offset % boot_sector.bytes_per_sector;
        auto sector          = io_.ReadSector(sector_number);
        auto &entry          = internal::get<ClusterNumT>(sector, sector_offset);
        entry                = (entry & ~ImplT::kClusterMask) | (value & ImplT::kClusterMask);
        io_.WriteSector(sector_number, sector);
    }

    NODISCARD FORCE_INLINE_F size_t GetClusterSize_() const
    {
        auto &bs = GetBootSector_();
        return bs.sectors_per_cluster * bs.bytes_per_sector;
    }

    // ------------------------------
    // Protected Data Members
    // ------------------------------

    io::SectorRange fat_region_;
    io::SectorRange root_dir_region_;
    io::SectorRange data_region_;
    size_t cluster_count_{};
    IO &io_;

    private:
    // ------------------------------
    // CRTP Helpers
    // ------------------------------

    NODISCARD FORCE_INLINE_F ImplT &GetImpl_() { return *static_cast<ImplT *>(this); }
    NODISCARD FORCE_INLINE_F const ImplT &GetImpl_() const
    {
        return *static_cast<const ImplT *>(this);
    }

    NODISCARD FORCE_INLINE_F const BootSector &GetBootSector_() const
    {
        return static_cast<const BootSector &>(GetImpl_().boot_sector_.fat);
    }

    NODISCARD FORCE_INLINE_F ClusterNumT GetRootCluster_() const
    {
        return GetImpl_().GetRootCluster();
    }

    // ------------------------------
    // Validation Helpers
    // ------------------------------

    NODISCARD static FORCE_INLINE_F bool ValidateBootSignature_(std::span<byte> sector_data)
    {
        return internal::get<u16>(sector_data, 510) == BootSector::kSignature;
    }

    NODISCARD static FORCE_INLINE_F bool ValidateJumpInstruction_(const BootSector &bs)
    {
        return (bs.jump_instruction[0] == 0xEB && bs.jump_instruction[2] == 0x90) ||
               bs.jump_instruction[0] == 0xE9;
    }

    NODISCARD static FORCE_INLINE_F bool ValidateSectorAndClusterSize_(const BootSector &bs)
    {
        if (!IsPowerOfTwo(bs.bytes_per_sector) || bs.bytes_per_sector < 512 ||
            bs.bytes_per_sector > 4096) {
            return false;
        }
        if (!IsPowerOfTwo(bs.sectors_per_cluster) || bs.sectors_per_cluster < 1 ||
            bs.sectors_per_cluster > 128) {
            return false;
        }
        return bs.bytes_per_sector * bs.sectors_per_cluster <= kMaxClusterSize;
    }

    NODISCARD static FORCE_INLINE_F bool ValidateReservedSectors_(const BootSector &bs)
    {
        return bs.reserved_sectors != 0;
    }

    NODISCARD static FORCE_INLINE_F bool ValidateFatCount_(const BootSector &bs)
    {
        return bs.number_of_fats != 0;
    }

    NODISCARD static FORCE_INLINE_F bool ValidateMediaDescriptor_(const BootSector &bs)
    {
        constexpr u8 kValid[] = {0xF0, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF};
        for (auto desc : kValid) {
            if (bs.media_descriptor == desc)
                return true;
        }
        return false;
    }

    NODISCARD static FORCE_INLINE_F bool ValidateFatMediaDescriptor_(IO &io, const BootSector &bs)
    {
        auto fat_region       = io.ReadSector(bs.reserved_sectors);
        ClusterNumT fat_entry = internal::get<ClusterNumT>(fat_region);
        return static_cast<u8>(fat_entry & 0xFF) == bs.media_descriptor;
    }

    // ------------------------------
    // Cluster Management
    // ------------------------------

    NODISCARD FORCE_INLINE_F ClusterNumT AllocateCluster_(ClusterNumT value = ImplT::kEOC)
    {
        auto &impl = GetImpl_();
        for (ClusterNumT cluster = kFirstClusterNumber;
             cluster < cluster_count_ + kFirstClusterNumber; ++cluster) {
            if (impl.GetFATEntry_(cluster) == 0) {
                impl.SetFATEntry_(cluster, value);
                return cluster;
            }
        }
        return 0;
    }

    FORCE_INLINE_F void FreeClusterChain_(ClusterNumT cluster)
    {
        auto &impl = GetImpl_();
        while (cluster != 0 && cluster < ImplT::kEOC) {
            ClusterNumT next = impl.GetFATEntry_(cluster);
            impl.SetFATEntry_(cluster, 0);
            cluster = next;
        }
    }

    NODISCARD FORCE_INLINE_F std::span<const byte> ReadCluster_(ClusterNumT cluster) const
    {
        return io_.ReadRange({GetClusterSector_(cluster), GetBootSector_().sectors_per_cluster});
    }

    FORCE_INLINE_F void WriteCluster_(ClusterNumT cluster, std::span<const byte> data)
    {
        io_.WriteRange({GetClusterSector_(cluster), GetBootSector_().sectors_per_cluster}, data);
    }

    // ------------------------------
    // Directory Scanning
    // ------------------------------

    template <typename Callback>
    FORCE_INLINE_F void ScanDirectoryTable_(
        const DirectoryEntry &dir_entry, Callback &&callback
    ) const
    {
        ClusterNumT cluster = GetFirstCluster_(dir_entry);

        if constexpr (kHasFixedRootDir) {
            if (cluster == 0) {
                ScanFixedRootDirectory_(callback);
                return;
            }
        }

        ScanClusterChainDirectory_(cluster, callback);
    }

    template <typename Callback>
    FORCE_INLINE_F void ScanFixedRootDirectory_(Callback &&callback) const
    {
        auto root_data     = io_.ReadRange(root_dir_region_);
        size_t entry_count = root_data.size() / sizeof(DirectoryEntry);

        for (size_t i = 0; i < entry_count; ++i) {
            const auto &entry =
                internal::get<DirectoryEntry>(root_data, i * sizeof(DirectoryEntry));
            if (entry.IsFree())
                continue;
            if (entry.IsFurtherEntriesFree())
                break;
            callback(entry);
        }
    }

    template <typename Callback>
    FORCE_INLINE_F void ScanClusterChainDirectory_(ClusterNumT cluster, Callback &&callback) const
    {
        auto &impl = GetImpl_();

        while (cluster < ImplT::kEOC) {
            auto cluster_data  = ReadCluster_(cluster);
            size_t entry_count = cluster_data.size() / sizeof(DirectoryEntry);

            for (size_t i = 0; i < entry_count; ++i) {
                const auto &entry =
                    internal::get<DirectoryEntry>(cluster_data, i * sizeof(DirectoryEntry));
                if (entry.IsFree())
                    continue;
                if (entry.IsFurtherEntriesFree())
                    return;
                callback(entry);
            }
            cluster = impl.GetFATEntry_(cluster);
        }
    }

    template <typename Callback>
    FORCE_INLINE_F void ScanDirectoryTableWithOffset_(
        const DirectoryEntry &dir_entry, Callback &&callback
    ) const
    {
        ClusterNumT cluster = GetFirstCluster_(dir_entry);

        if constexpr (kHasFixedRootDir) {
            if (cluster == 0) {
                ScanFixedRootDirectoryWithOffset_(callback);
                return;
            }
        }
        ScanClusterChainDirectoryWithOffset_(cluster, callback);
    }

    template <typename Callback>
    FORCE_INLINE_F void ScanFixedRootDirectoryWithOffset_(Callback &&callback) const
    {
        auto root_data     = io_.ReadRange(root_dir_region_);
        size_t entry_count = root_data.size() / sizeof(DirectoryEntry);
        size_t offset      = 0;

        for (size_t i = 0; i < entry_count; ++i, offset += sizeof(DirectoryEntry)) {
            const auto &entry =
                internal::get<DirectoryEntry>(root_data, i * sizeof(DirectoryEntry));
            if (entry.IsFree())
                continue;
            if (entry.IsFurtherEntriesFree())
                break;
            if (!callback(entry, offset))
                break;
        }
    }

    template <typename Callback>
    FORCE_INLINE_F void ScanClusterChainDirectoryWithOffset_(
        ClusterNumT cluster, Callback &&callback
    ) const
    {
        auto &impl           = GetImpl_();
        size_t global_offset = 0;

        while (cluster < ImplT::kEOC) {
            auto cluster_data  = ReadCluster_(cluster);
            size_t entry_count = cluster_data.size() / sizeof(DirectoryEntry);

            for (size_t i = 0; i < entry_count; ++i) {
                const auto &entry =
                    internal::get<DirectoryEntry>(cluster_data, i * sizeof(DirectoryEntry));

                if (entry.IsFree()) {
                    global_offset += sizeof(DirectoryEntry);
                    continue;
                }
                if (entry.IsFurtherEntriesFree())
                    return;
                if (!callback(entry, global_offset))
                    return;
                global_offset += sizeof(DirectoryEntry);
            }
            cluster = impl.GetFATEntry_(cluster);
        }
    }

    // ------------------------------
    // Path Lookup
    // ------------------------------

    NODISCARD PathLookupResult LookupPath_(const Path &path)
    {
        PathLookupResult result{};
        result.found = false;

        if (path.IsEmpty() || path.IsRoot()) {
            return result;
        }

        DirectoryEntry current_entry = MakeRootDirectoryEntry_();
        ClusterNumT current_cluster  = GetRootCluster_();
        ClusterNumT parent_cluster   = current_cluster;

        for (size_t i = 0; i < path.ComponentCount(); ++i) {
            auto component = path.GetComponent(i);

            if (component == ".")
                continue;
            if (component == "..")
                continue;

            auto match = FindEntryInDirectory_(current_entry, component);
            if (!match.found)
                return result;

            parent_cluster      = current_cluster;
            current_entry       = match.entry;
            current_cluster     = GetFirstCluster_(current_entry);
            result.entry_offset = match.entry_offset;
        }

        result.entry          = current_entry;
        result.parent_cluster = parent_cluster;
        result.found          = true;
        return result;
    }

    NODISCARD FORCE_INLINE_F DirectoryEntry MakeRootDirectoryEntry_() const
    {
        DirectoryEntry entry{};
        ClusterNumT root         = GetRootCluster_();
        entry.attributes         = DirectoryEntry::Attributes::Directory;
        entry.first_cluster_low  = root & 0xFFFF;
        entry.first_cluster_high = (root >> 16) & 0xFFFF;
        return entry;
    }

    struct EntryMatch {
        DirectoryEntry entry;
        size_t entry_offset;
        bool found;
    };

    NODISCARD EntryMatch
    FindEntryInDirectory_(const DirectoryEntry &dir_entry, std::string_view component)
    {
        EntryMatch match{};
        match.found = false;

        char formatted[kMaxNameLength];
        char component_buf[kMaxComponentSize];
        size_t copy_len = std::min(component.size(), kMaxComponentSize - 1);
        memcpy(component_buf, component.data(), copy_len);
        component_buf[copy_len] = '\0';

        if (!ToShortFormat_(component_buf, formatted)) {
            return match;
        }

        ScanDirectoryTableWithOffset_(dir_entry, [&](const DirectoryEntry &entry, size_t offset) {
            if (!entry.IsLongNameSubComponent() && !entry.IsFree()) {
                if (memcmp(entry.filename, formatted, kMaxNameLength) == 0) {
                    match.entry        = entry;
                    match.entry_offset = offset;
                    match.found        = true;
                    return false;
                }
            }
            return true;
        });

        return match;
    }

    NODISCARD PathLookupResult GetDirectoryLookup_(const Path &path)
    {
        PathLookupResult lookup;
        if (path.IsRoot()) {
            lookup.found = true;
            lookup.entry = MakeRootDirectoryEntry_();
        } else {
            lookup = LookupPath_(path);
        }
        return lookup;
    }

    NODISCARD std::expected<ClusterNumT, VfsError> GetParentCluster_(const Path &path)
    {
        auto parent_path = path.GetParent();
        if (parent_path.IsRoot()) {
            return GetRootCluster_();
        }
        auto parent_lookup = LookupPath_(parent_path);
        if (!parent_lookup.found) {
            return std::unexpected(VfsError::kDirectoryNotFound);
        }
        return GetFirstCluster_(parent_lookup.entry);
    }

    // ------------------------------
    // File Data Operations
    // ------------------------------

    NODISCARD size_t
    ReadFileData_(const DirectoryEntry &entry, void *buffer, size_t size, size_t offset)
    {
        size_t bytes_to_read     = std::min(size, static_cast<size_t>(entry.file_size - offset));
        size_t cluster_size      = GetClusterSize_();
        ClusterNumT cluster      = SkipClusters_(GetFirstCluster_(entry), offset / cluster_size);
        size_t offset_in_cluster = offset % cluster_size;

        auto *dst         = static_cast<byte *>(buffer);
        size_t bytes_read = 0;

        while (bytes_read < bytes_to_read && cluster < ImplT::kEOC) {
            auto cluster_data = ReadCluster_(cluster);
            size_t copy_size =
                std::min(cluster_size - offset_in_cluster, bytes_to_read - bytes_read);
            memcpy(dst + bytes_read, cluster_data.data() + offset_in_cluster, copy_size);

            bytes_read += copy_size;
            offset_in_cluster = 0;
            cluster           = GetImpl_().GetFATEntry_(cluster);
        }

        return bytes_read;
    }

    NODISCARD Result<size_t> WriteFileData_(
        PathLookupResult &lookup, const void *buffer, size_t size, size_t offset
    )
    {
        size_t cluster_size = GetClusterSize_();
        ClusterNumT cluster = GetFirstCluster_(lookup.entry);

        if (cluster == 0) {
            cluster = AllocateCluster_();
            if (cluster == 0)
                return std::unexpected(VfsError::kDiskFull);
            SetEntryFirstCluster_(lookup.entry, cluster);
            UpdateDirectoryEntry_(lookup.parent_cluster, lookup.entry_offset, lookup.entry);
        }

        cluster = EnsureClusterChain_(cluster, offset / cluster_size);
        if (cluster == 0)
            return std::unexpected(VfsError::kDiskFull);

        size_t bytes_written = WriteToClusterChain_(cluster, buffer, size, offset % cluster_size);

        size_t new_size =
            std::max(static_cast<size_t>(lookup.entry.file_size), offset + bytes_written);
        if (new_size != lookup.entry.file_size) {
            lookup.entry.file_size = static_cast<u32>(new_size);
            UpdateDirectoryEntry_(lookup.parent_cluster, lookup.entry_offset, lookup.entry);
        }

        return bytes_written;
    }

    NODISCARD FORCE_INLINE_F ClusterNumT SkipClusters_(ClusterNumT cluster, size_t count)
    {
        for (size_t i = 0; i < count && cluster < ImplT::kEOC; ++i) {
            cluster = GetImpl_().GetFATEntry_(cluster);
        }
        return cluster;
    }

    NODISCARD FORCE_INLINE_F ClusterNumT
    EnsureClusterChain_(ClusterNumT cluster, size_t clusters_needed)
    {
        for (size_t i = 0; i < clusters_needed; ++i) {
            ClusterNumT next = GetImpl_().GetFATEntry_(cluster);
            if (next >= ImplT::kEOC) {
                next = AllocateCluster_();
                if (next == 0)
                    return 0;
                GetImpl_().SetFATEntry_(cluster, next);
            }
            cluster = next;
        }
        return cluster;
    }

    NODISCARD size_t WriteToClusterChain_(
        ClusterNumT cluster, const void *buffer, size_t size, size_t offset_in_cluster
    )
    {
        size_t cluster_size  = GetClusterSize_();
        const auto *src      = static_cast<const byte *>(buffer);
        size_t bytes_written = 0;

        while (bytes_written < size) {
            auto cluster_data = ReadCluster_(cluster);
            byte temp_buffer[kMaxClusterSize];
            memcpy(temp_buffer, cluster_data.data(), cluster_size);

            size_t copy_size = std::min(cluster_size - offset_in_cluster, size - bytes_written);
            memcpy(temp_buffer + offset_in_cluster, src + bytes_written, copy_size);
            WriteCluster_(cluster, std::span<const byte>(temp_buffer, cluster_size));

            bytes_written += copy_size;
            offset_in_cluster = 0;

            if (bytes_written < size) {
                ClusterNumT next = GetImpl_().GetFATEntry_(cluster);
                if (next >= ImplT::kEOC) {
                    next = AllocateCluster_();
                    if (next == 0)
                        break;
                    GetImpl_().SetFATEntry_(cluster, next);
                }
                cluster = next;
            }
        }

        return bytes_written;
    }

    FAST_CALL void SetEntryFirstCluster_(DirectoryEntry &entry, ClusterNumT cluster)
    {
        entry.first_cluster_low  = cluster & 0xFFFF;
        entry.first_cluster_high = (cluster >> 16) & 0xFFFF;
    }

    // ------------------------------
    // Directory Entry Management
    // ------------------------------

    NODISCARD Result<> DeleteEntry_(PathLookupResult &lookup)
    {
        FreeClusterChain_(GetFirstCluster_(lookup.entry));
        reinterpret_cast<byte &>(lookup.entry) = DirectoryEntry::kDeletedEntry;
        UpdateDirectoryEntry_(lookup.parent_cluster, lookup.entry_offset, lookup.entry);
        return {};
    }

    NODISCARD Result<> MoveEntry_(
        PathLookupResult &old_lookup, const char (&new_name)[kMaxNameLength],
        ClusterNumT new_parent_cluster
    )
    {
        if (old_lookup.parent_cluster == new_parent_cluster) {
            memcpy(old_lookup.entry.filename, new_name, kMaxNameLength);
            UpdateDirectoryEntry_(
                old_lookup.parent_cluster, old_lookup.entry_offset, old_lookup.entry
            );
            return {};
        }

        auto result = CreateEntry_(
            new_parent_cluster, new_name, old_lookup.entry.attributes,
            GetFirstCluster_(old_lookup.entry)
        );
        RET_UNEXPECTED_IF_ERR(result);

        old_lookup.entry.filename[0] = DirectoryEntry::kDeletedEntry;
        UpdateDirectoryEntry_(old_lookup.parent_cluster, old_lookup.entry_offset, old_lookup.entry);
        return {};
    }

    NODISCARD Result<> CreateEntry_(
        ClusterNumT parent_cluster, const char (&name)[kMaxNameLength],
        DirectoryEntry::Attributes attributes, ClusterNumT first_cluster
    )
    {
        if constexpr (kHasFixedRootDir) {
            if (parent_cluster == 0) {
                return CreateEntryInFixedRootDir_(name, attributes, first_cluster);
            }
        }
        return CreateEntryInClusterDir_(parent_cluster, name, attributes, first_cluster);
    }

    NODISCARD Result<> CreateEntryInFixedRootDir_(
        const char (&name)[kMaxNameLength], DirectoryEntry::Attributes attributes,
        ClusterNumT first_cluster
    )
    {
        auto root_data     = io_.ReadRange(root_dir_region_);
        size_t root_size   = root_dir_region_.count * GetBootSector_().bytes_per_sector;
        size_t entry_count = root_size / sizeof(DirectoryEntry);

        for (size_t i = 0; i < entry_count; ++i) {
            const auto &entry =
                internal::get<DirectoryEntry>(root_data, i * sizeof(DirectoryEntry));

            if (entry.IsFree() || entry.IsFurtherEntriesFree()) {
                byte temp_buffer[kMaxClusterSize];
                memcpy(temp_buffer, root_data.data(), root_size);
                WriteNewEntry_(
                    temp_buffer, i, name, attributes, first_cluster, entry.IsFurtherEntriesFree(),
                    i + 1 < entry_count
                );
                io_.WriteRange(root_dir_region_, std::span<const byte>(temp_buffer, root_size));
                return {};
            }
        }
        return std::unexpected(VfsError::kDiskFull);
    }

    NODISCARD Result<> CreateEntryInClusterDir_(
        ClusterNumT parent_cluster, const char (&name)[kMaxNameLength],
        DirectoryEntry::Attributes attributes, ClusterNumT first_cluster
    )
    {
        size_t cluster_size = GetClusterSize_();
        auto &impl          = GetImpl_();
        ClusterNumT cluster = parent_cluster;

        while (cluster < ImplT::kEOC) {
            auto cluster_data  = ReadCluster_(cluster);
            size_t entry_count = cluster_data.size() / sizeof(DirectoryEntry);

            for (size_t i = 0; i < entry_count; ++i) {
                const auto &entry =
                    internal::get<DirectoryEntry>(cluster_data, i * sizeof(DirectoryEntry));

                if (entry.IsFree() || entry.IsFurtherEntriesFree()) {
                    byte temp_buffer[kMaxClusterSize];
                    memcpy(temp_buffer, cluster_data.data(), cluster_size);
                    WriteNewEntry_(
                        temp_buffer, i, name, attributes, first_cluster,
                        entry.IsFurtherEntriesFree(), i + 1 < entry_count
                    );
                    WriteCluster_(cluster, std::span<const byte>(temp_buffer, cluster_size));
                    return {};
                }
            }

            ClusterNumT next = impl.GetFATEntry_(cluster);
            if (next >= ImplT::kEOC) {
                next = AllocateCluster_();
                if (next == 0)
                    return std::unexpected(VfsError::kDiskFull);
                impl.SetFATEntry_(cluster, next);
                InitializeEmptyCluster_(next);
            }
            cluster = next;
        }

        return std::unexpected(VfsError::kDiskFull);
    }

    FORCE_INLINE_F void WriteNewEntry_(
        byte *buffer, size_t index, const char (&name)[kMaxNameLength],
        DirectoryEntry::Attributes attributes, ClusterNumT first_cluster, bool was_eod,
        bool has_next_slot
    )
    {
        auto *new_entry =
            reinterpret_cast<DirectoryEntry *>(buffer + index * sizeof(DirectoryEntry));
        memcpy(new_entry->filename, name, kMaxNameLength);
        new_entry->attributes          = attributes;
        new_entry->creation_time_tenth = 0;
        new_entry->creation_time       = 0;
        new_entry->creation_date       = 0;
        new_entry->last_access_date    = 0;
        new_entry->first_cluster_high  = (first_cluster >> 16) & 0xFFFF;
        new_entry->last_write_time     = 0;
        new_entry->last_write_date     = 0;
        new_entry->first_cluster_low   = first_cluster & 0xFFFF;
        new_entry->file_size           = 0;

        if (was_eod && has_next_slot) {
            *(buffer + (index + 1) * sizeof(DirectoryEntry)) = DirectoryEntry::kEOD;
        }
    }

    FORCE_INLINE_F void InitializeEmptyCluster_(ClusterNumT cluster)
    {
        size_t cluster_size = GetClusterSize_();
        byte data[kMaxClusterSize];
        memset(data, 0, cluster_size);
        data[0] = DirectoryEntry::kEOD;
        WriteCluster_(cluster, std::span<const byte>(data, cluster_size));
    }

    void UpdateDirectoryEntry_(
        ClusterNumT parent_cluster, size_t offset, const DirectoryEntry &entry
    )
    {
        if constexpr (kHasFixedRootDir) {
            if (parent_cluster == 0) {
                UpdateEntryInFixedRootDir_(offset, entry);
                return;
            }
        }
        UpdateEntryInClusterDir_(parent_cluster, offset, entry);
    }

    FORCE_INLINE_F void UpdateEntryInFixedRootDir_(size_t offset, const DirectoryEntry &entry)
    {
        size_t root_size = root_dir_region_.count * GetBootSector_().bytes_per_sector;
        if (offset >= root_size)
            return;

        auto root_data = io_.ReadRange(root_dir_region_);
        byte temp_buffer[kMaxClusterSize];
        memcpy(temp_buffer, root_data.data(), root_size);
        memcpy(temp_buffer + offset, &entry, sizeof(DirectoryEntry));
        io_.WriteRange(root_dir_region_, std::span<const byte>(temp_buffer, root_size));
    }

    FORCE_INLINE_F void UpdateEntryInClusterDir_(
        ClusterNumT parent_cluster, size_t offset, const DirectoryEntry &entry
    )
    {
        size_t cluster_size = GetClusterSize_();
        auto &impl          = GetImpl_();
        ClusterNumT cluster = parent_cluster;

        while (offset >= cluster_size && cluster < ImplT::kEOC) {
            offset -= cluster_size;
            cluster = impl.GetFATEntry_(cluster);
        }

        if (cluster >= ImplT::kEOC)
            return;

        auto cluster_data = ReadCluster_(cluster);
        byte temp_buffer[kMaxClusterSize];
        memcpy(temp_buffer, cluster_data.data(), cluster_size);
        memcpy(temp_buffer + offset, &entry, sizeof(DirectoryEntry));
        WriteCluster_(cluster, std::span<const byte>(temp_buffer, cluster_size));
    }

    // ------------------------------
    // Directory Creation Helpers
    // ------------------------------

    NODISCARD Result<> CreateDirectoryWithCluster_(
        ClusterNumT parent_cluster, const char (&name)[kMaxNameLength]
    )
    {
        ClusterNumT new_cluster = AllocateCluster_();
        if (new_cluster == 0) {
            return std::unexpected(VfsError::kDiskFull);
        }

        InitializeNewDirectory_(new_cluster, parent_cluster);
        return CreateEntry_(
            parent_cluster, name, DirectoryEntry::Attributes::Directory, new_cluster
        );
    }

    FORCE_INLINE_F void InitializeNewDirectory_(ClusterNumT new_cluster, ClusterNumT parent_cluster)
    {
        size_t cluster_size = GetClusterSize_();
        byte dir_data[kMaxClusterSize];
        memset(dir_data, 0, cluster_size);

        auto *dot_entry = reinterpret_cast<DirectoryEntry *>(dir_data);
        memcpy(dot_entry->filename, kDot, kMaxNameLength);
        dot_entry->attributes         = DirectoryEntry::Attributes::Directory;
        dot_entry->first_cluster_low  = new_cluster & 0xFFFF;
        dot_entry->first_cluster_high = (new_cluster >> 16) & 0xFFFF;

        auto *dotdot_entry = reinterpret_cast<DirectoryEntry *>(dir_data + sizeof(DirectoryEntry));
        memcpy(dotdot_entry->filename, kDotDot, kMaxNameLength);
        dotdot_entry->attributes         = DirectoryEntry::Attributes::Directory;
        dotdot_entry->first_cluster_low  = parent_cluster & 0xFFFF;
        dotdot_entry->first_cluster_high = (parent_cluster >> 16) & 0xFFFF;

        WriteCluster_(new_cluster, std::span<const byte>(dir_data, cluster_size));
    }

    NODISCARD FORCE_INLINE_F bool IsDirectoryEmpty_(const DirectoryEntry &entry) const
    {
        bool is_empty = true;
        ScanDirectoryTable_(entry, [&is_empty](const DirectoryEntry &e) {
            if (!e.IsFree() && !e.IsLongNameSubComponent() && !e.IsVolumeLabel()) {
                if (memcmp(e.filename, kDot, kMaxNameLength) != 0 &&
                    memcmp(e.filename, kDotDot, kMaxNameLength) != 0) {
                    is_empty = false;
                }
            }
        });
        return is_empty;
    }

    // ------------------------------
    // Name Formatting
    // ------------------------------

    NODISCARD FAST_CALL bool FormatFilename_(
        std::string_view filename, char (&formatted)[kMaxNameLength]
    )
    {
        char buf[kMaxComponentSize];
        size_t len = std::min(filename.size(), kMaxComponentSize - 1);
        memcpy(buf, filename.data(), len);
        buf[len] = '\0';
        return ToShortFormat_(buf, formatted);
    }

    NODISCARD FAST_CALL bool ToShortFormat_(const char *filename, char (&formatted)[kMaxNameLength])
    {
        if (!filename)
            return false;

        const size_t len = strlen(filename);
        if (len > kMaxNameLength + 1)
            return false;

        const char *dot_pos   = strrchr(filename, '.');
        const size_t name_len = dot_pos ? static_cast<size_t>(dot_pos - filename) : len;
        const size_t ext_len  = dot_pos ? (len - name_len - 1) : 0;

        if (ext_len > 3 || name_len == 0 || name_len > 8)
            return false;

        memset(formatted, ' ', kMaxNameLength);

        if (!CopyAndValidateChars_(filename, formatted, name_len, 0))
            return false;
        if (dot_pos && !CopyAndValidateChars_(dot_pos + 1, formatted, ext_len, 8))
            return false;

        return true;
    }

    NODISCARD FAST_CALL bool CopyAndValidateChars_(
        const char *src, char *dst, size_t len, size_t dst_offset
    )
    {
        constexpr u8 kInvalidChars[] = {0x22, 0x2A, 0x2B, 0x2C, 0x2E, 0x2F, 0x3A, 0x3B,
                                        0x3C, 0x3D, 0x3E, 0x3F, 0x5B, 0x5C, 0x5D, 0x7C};

        for (size_t i = 0; i < len; ++i) {
            char c = src[i];
            if (c < 0x20 && c != 0x05)
                return false;

            for (auto invalid : kInvalidChars) {
                if (static_cast<u8>(c) == invalid)
                    return false;
            }

            dst[dst_offset + i] =
                (c == 0x05) ? static_cast<char>(toupper(0xE5)) : static_cast<char>(toupper(c));
        }
        return true;
    }

    FAST_CALL void FormatShortName_(const char (&short_name)[11], char *output)
    {
        size_t out_idx = 0;

        for (size_t i = 0; i < 8 && short_name[i] != ' '; ++i) {
            output[out_idx++] = short_name[i];
        }

        if (short_name[8] != ' ') {
            output[out_idx++] = '.';
            for (size_t i = 8; i < 11 && short_name[i] != ' '; ++i) {
                output[out_idx++] = short_name[i];
            }
        }

        output[out_idx] = '\0';
    }

    // ------------------------------
    // Static Callback Wrappers
    // ------------------------------

    FAST_CALL Result<> CreateFileCallback_(void *ctx, const Path &path)
    {
        return static_cast<Fat *>(ctx)->CreateFile(path);
    }

    WRAP_CALL Result<size_t> ReadFileCallback_(
        void *ctx, const Path &path, void *buffer, size_t size, size_t offset
    )
    {
        return static_cast<Fat *>(ctx)->ReadFile(path, buffer, size, offset);
    }

    WRAP_CALL Result<size_t> WriteFileCallback_(
        void *ctx, const Path &path, const void *buffer, size_t size, size_t offset
    )
    {
        return static_cast<Fat *>(ctx)->WriteFile(path, buffer, size, offset);
    }

    WRAP_CALL Result<> DeleteFileCallback_(void *ctx, const Path &path)
    {
        return static_cast<Fat *>(ctx)->DeleteFile(path);
    }

    WRAP_CALL Result<bool> FileExistsCallback_(void *ctx, const Path &path)
    {
        return static_cast<Fat *>(ctx)->FileExists(path);
    }

    WRAP_CALL Result<size_t> GetFileSizeCallback_(void *ctx, const Path &path)
    {
        return static_cast<Fat *>(ctx)->GetFileSize(path);
    }

    WRAP_CALL Result<> CreateDirectoryCallback_(void *ctx, const Path &path)
    {
        return static_cast<Fat *>(ctx)->CreateDirectory(path);
    }

    WRAP_CALL Result<> RemoveDirectoryCallback_(void *ctx, const Path &path)
    {
        return static_cast<Fat *>(ctx)->RemoveDirectory(path);
    }

    WRAP_CALL void ListDirectoryCallback_(
        void *ctx, const Path &path, ListDirCallback callback, void *user_ctx
    )
    {
        static_cast<Fat *>(ctx)->ListDirectory(path, callback, user_ctx);
    }

    WRAP_CALL Result<bool> DirectoryExistsCallback_(void *ctx, const Path &path)
    {
        return static_cast<Fat *>(ctx)->DirectoryExists(path);
    }

    WRAP_CALL Result<bool> ExistsCallback_(void *ctx, const Path &path)
    {
        return static_cast<Fat *>(ctx)->Exists(path);
    }

    WRAP_CALL Result<> MoveCallback_(void *ctx, const Path &old_path, const Path &new_path)
    {
        return static_cast<Fat *>(ctx)->Move(old_path, new_path);
    }
};

}  // namespace vfs

#endif  // KERNEL_SRC_VFS_FAT_FAT_HPP_
