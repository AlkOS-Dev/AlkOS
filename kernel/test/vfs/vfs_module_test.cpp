#include <test_module/test.hpp>

#include <string.h>
#include <mem/heap.hpp>
#include <memory.hpp>
#include <modules/vfs.hpp>
#include <vfs/fat/fat12.hpp>
#include <vfs/io/in_memory.hpp>
#include <vfs/path.hpp>

class VfsFat12ImageHelper
{
    public:
    static constexpr size_t kSectorSize        = 512;
    static constexpr size_t kSectorsPerCluster = 1;
    static constexpr size_t kReservedSectors   = 1;
    static constexpr size_t kNumberOfFats      = 2;
    static constexpr size_t kRootDirEntries    = 16;
    static constexpr size_t kFatSizeSectors    = 1;
    static constexpr size_t kTotalSectors      = 64;

    static constexpr size_t kFatRegionStart = kReservedSectors;
    static constexpr size_t kRootDirStart   = kReservedSectors + kNumberOfFats * kFatSizeSectors;
    static constexpr size_t kRootDirSectors =
        (kRootDirEntries * 32 + kSectorSize - 1) / kSectorSize;
    static constexpr size_t kDataRegionStart = kRootDirStart + kRootDirSectors;
    static constexpr size_t kImageSize       = kTotalSectors * kSectorSize;

    static void CreateMinimalImage(byte *image)
    {
        memset(image, 0, kImageSize);
        CreateBootSector(image);
        CreateFatTables(image);
        CreateRootDirectory(image);
    }

    private:
    static void CreateBootSector(byte *image)
    {
        image[0] = 0xEB;
        image[1] = 0x3C;
        image[2] = 0x90;
        memcpy(image + 3, "MSDOS5.0", 8);
        *reinterpret_cast<u16 *>(image + 11) = kSectorSize;
        image[13]                            = kSectorsPerCluster;
        *reinterpret_cast<u16 *>(image + 14) = kReservedSectors;
        image[16]                            = kNumberOfFats;
        *reinterpret_cast<u16 *>(image + 17) = kRootDirEntries;
        *reinterpret_cast<u16 *>(image + 19) = kTotalSectors;
        image[21]                            = 0xF8;
        *reinterpret_cast<u16 *>(image + 22) = kFatSizeSectors;
        *reinterpret_cast<u16 *>(image + 24) = 18;
        *reinterpret_cast<u16 *>(image + 26) = 2;
        *reinterpret_cast<u32 *>(image + 28) = 0;
        *reinterpret_cast<u32 *>(image + 32) = 0;
        image[36]                            = 0x80;
        image[38]                            = 0x29;
        *reinterpret_cast<u32 *>(image + 39) = 0x12345678;
        memcpy(image + 43, "TEST VOLUME", 11);
        memcpy(image + 54, "FAT12   ", 8);
        image[510] = 0x55;
        image[511] = 0xAA;
    }

    static void CreateFatTables(byte *image)
    {
        byte *fat1 = image + kFatRegionStart * kSectorSize;
        memset(fat1, 0, kFatSizeSectors * kSectorSize);
        fat1[0]    = 0xF8;
        fat1[1]    = 0xFF;
        fat1[2]    = 0xFF;
        byte *fat2 = image + (kFatRegionStart + kFatSizeSectors) * kSectorSize;
        memcpy(fat2, fat1, kFatSizeSectors * kSectorSize);
    }

    static void CreateRootDirectory(byte *image)
    {
        byte *root_dir = image + kRootDirStart * kSectorSize;
        memset(root_dir, 0, kRootDirSectors * kSectorSize);
        memcpy(root_dir, "TEST VOLUME", 11);
        root_dir[11] = 0x08;
    }
};

class VfsModuleTest : public TestGroupBase
{
    protected:
    alignas(16) byte disk_image_[VfsFat12ImageHelper::kImageSize];
    vfs::io::InMemory *io_{nullptr};
    vfs::Fat12<vfs::io::InMemory> *fat12_{nullptr};

    void Setup_() override
    {
        VfsFat12ImageHelper::CreateMinimalImage(disk_image_);

        io_ = Mem::KMalloc<vfs::io::InMemory>().value_or(nullptr);
        EXPECT_NOT_NULL(io_);
        std::construct_at<vfs::io::InMemory>(io_, disk_image_, VfsFat12ImageHelper::kSectorSize);

        fat12_ = Mem::KMalloc<vfs::Fat12<vfs::io::InMemory>>().value_or(nullptr);
        EXPECT_NOT_NULL(fat12_);
        std::construct_at<vfs::Fat12<vfs::io::InMemory>>(fat12_, *io_);

        vfs::Unmount(vfs::Path("/"));
    }

    void TearDown_() override
    {
        vfs::Unmount(vfs::Path("/"));

        std::destroy_at(fat12_);
        Mem::KFree(fat12_);

        std::destroy_at(io_);
        Mem::KFree(io_);
    }
};

// =============================================================================
// Mount/Unmount Tests
// =============================================================================

TEST_F(VfsModuleTest, MountFilesystemAtRoot)
{
    auto fs     = fat12_->GetFilesystem();
    auto result = vfs::Mount(vfs::Path("/"), {}, fs);
    EXPECT_TRUE(result.has_value());
}

TEST_F(VfsModuleTest, MountFailsForEmptyPath)
{
    auto fs     = fat12_->GetFilesystem();
    auto result = vfs::Mount(vfs::Path(""), {}, fs);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(vfs::VfsError::kInvalidPath, result.error());
}

TEST_F(VfsModuleTest, MountFailsForRelativePath)
{
    auto fs     = fat12_->GetFilesystem();
    auto result = vfs::Mount(vfs::Path("relative/path"), {}, fs);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(vfs::VfsError::kInvalidPath, result.error());
}

TEST_F(VfsModuleTest, MountFailsIfAlreadyMounted)
{
    auto fs = fat12_->GetFilesystem();

    auto result1 = vfs::Mount(vfs::Path("/"), {}, fs);
    EXPECT_TRUE(result1.has_value());

    auto result2 = vfs::Mount(vfs::Path("/"), {}, fs);
    EXPECT_FALSE(result2.has_value());
    EXPECT_EQ(vfs::VfsError::kAlreadyMounted, result2.error());
}

TEST_F(VfsModuleTest, UnmountSucceeds)
{
    auto fs = fat12_->GetFilesystem();
    vfs::Mount(vfs::Path("/"), {}, fs);

    auto result = vfs::Unmount(vfs::Path("/"));
    EXPECT_TRUE(result.has_value());
}

TEST_F(VfsModuleTest, UnmountFailsIfNotMounted)
{
    auto result = vfs::Unmount(vfs::Path("/mnt/notmounted"));
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(vfs::VfsError::kNotMounted, result.error());
}

// =============================================================================
// File Operations Through VFS
// =============================================================================

TEST_F(VfsModuleTest, CreateFileViaVfs)
{
    auto fs = fat12_->GetFilesystem();
    vfs::Mount(vfs::Path("/"), {}, fs);

    auto result = vfs::CreateFile(vfs::Path("/TEST.TXT"));
    EXPECT_TRUE(result.has_value());
}

TEST_F(VfsModuleTest, FileExistsViaVfs)
{
    auto fs = fat12_->GetFilesystem();
    vfs::Mount(vfs::Path("/"), {}, fs);

    vfs::CreateFile(vfs::Path("/EXISTS.TXT"));

    auto result = vfs::FileExists(vfs::Path("/EXISTS.TXT"));
    EXPECT_TRUE(result.has_value());
    EXPECT_TRUE(result.value());
}

TEST_F(VfsModuleTest, FileNotExistsViaVfs)
{
    auto fs = fat12_->GetFilesystem();
    vfs::Mount(vfs::Path("/"), {}, fs);

    auto result = vfs::FileExists(vfs::Path("/NOTHERE.TXT"));
    EXPECT_TRUE(result.has_value());
    EXPECT_FALSE(result.value());
}

TEST_F(VfsModuleTest, WriteAndReadFileViaVfs)
{
    auto fs = fat12_->GetFilesystem();
    vfs::Mount(vfs::Path("/"), {}, fs);

    vfs::Path path("/DATA.TXT");
    vfs::CreateFile(path);

    const char *test_data = "Hello VFS!";
    size_t data_len       = strlen(test_data);

    auto write_result = vfs::WriteFile(path, test_data, data_len);
    EXPECT_TRUE(write_result.has_value());
    EXPECT_EQ(data_len, write_result.value());

    char buffer[64]  = {0};
    auto read_result = vfs::ReadFile(path, buffer, sizeof(buffer));
    EXPECT_TRUE(read_result.has_value());
    EXPECT_EQ(data_len, read_result.value());
    EXPECT_STREQ(test_data, buffer);
}

TEST_F(VfsModuleTest, DeleteFileViaVfs)
{
    auto fs = fat12_->GetFilesystem();
    vfs::Mount(vfs::Path("/"), {}, fs);

    vfs::Path path("/TODEL.TXT");
    vfs::CreateFile(path);

    auto delete_result = vfs::DeleteFile(path);
    EXPECT_TRUE(delete_result.has_value());

    auto exists_result = vfs::FileExists(path);
    EXPECT_TRUE(exists_result.has_value());
    EXPECT_FALSE(exists_result.value());
}

TEST_F(VfsModuleTest, GetFileSizeViaVfs)
{
    auto fs = fat12_->GetFilesystem();
    vfs::Mount(vfs::Path("/"), {}, fs);

    vfs::Path path("/SIZE.TXT");
    vfs::CreateFile(path);

    const char *data = "1234567890";
    vfs::WriteFile(path, data, 10);

    auto result = vfs::GetFileSize(path);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(10u, result.value());
}

// =============================================================================
// Directory Operations Through VFS
// =============================================================================

TEST_F(VfsModuleTest, CreateDirectoryViaVfs)
{
    auto fs = fat12_->GetFilesystem();
    vfs::Mount(vfs::Path("/"), {}, fs);

    auto result = vfs::CreateDirectory(vfs::Path("/TESTDIR"));
    EXPECT_TRUE(result.has_value());
}

TEST_F(VfsModuleTest, DirectoryExistsViaVfs)
{
    auto fs = fat12_->GetFilesystem();
    vfs::Mount(vfs::Path("/"), {}, fs);

    vfs::CreateDirectory(vfs::Path("/MYDIR"));

    auto result = vfs::DirectoryExists(vfs::Path("/MYDIR"));
    EXPECT_TRUE(result.has_value());
    EXPECT_TRUE(result.value());
}

TEST_F(VfsModuleTest, RootDirectoryExists)
{
    auto fs = fat12_->GetFilesystem();
    vfs::Mount(vfs::Path("/"), {}, fs);

    auto result = vfs::DirectoryExists(vfs::Path("/"));
    EXPECT_TRUE(result.has_value());
    EXPECT_TRUE(result.value());
}

TEST_F(VfsModuleTest, RemoveDirectoryViaVfs)
{
    auto fs = fat12_->GetFilesystem();
    vfs::Mount(vfs::Path("/"), {}, fs);

    vfs::Path path("/RMDIR");
    vfs::CreateDirectory(path);

    auto remove_result = vfs::RemoveDirectory(path);
    EXPECT_TRUE(remove_result.has_value());

    auto exists_result = vfs::DirectoryExists(path);
    EXPECT_TRUE(exists_result.has_value());
    EXPECT_FALSE(exists_result.value());
}

TEST_F(VfsModuleTest, ListDirectoryViaVfs)
{
    auto fs = fat12_->GetFilesystem();
    vfs::Mount(vfs::Path("/"), {}, fs);

    vfs::CreateFile(vfs::Path("/FILE1.TXT"));
    vfs::CreateFile(vfs::Path("/FILE2.TXT"));
    vfs::CreateDirectory(vfs::Path("/SUBDIR"));

    int file_count = 0;
    int dir_count  = 0;

    vfs::ListDirectory(vfs::Path("/"), [&](const char *, bool is_dir) {
        if (is_dir) {
            ++dir_count;
        } else {
            ++file_count;
        }
    });

    EXPECT_EQ(2, file_count);
    EXPECT_EQ(1, dir_count);
}

// =============================================================================
// General Operations Through VFS
// =============================================================================

TEST_F(VfsModuleTest, ExistsForFile)
{
    auto fs = fat12_->GetFilesystem();
    vfs::Mount(vfs::Path("/"), {}, fs);

    vfs::CreateFile(vfs::Path("/FILE.TXT"));

    auto result = vfs::Exists(vfs::Path("/FILE.TXT"));
    EXPECT_TRUE(result.has_value());
    EXPECT_TRUE(result.value());
}

TEST_F(VfsModuleTest, ExistsForDirectory)
{
    auto fs = fat12_->GetFilesystem();
    vfs::Mount(vfs::Path("/"), {}, fs);

    vfs::CreateDirectory(vfs::Path("/DIR"));

    auto result = vfs::Exists(vfs::Path("/DIR"));
    EXPECT_TRUE(result.has_value());
    EXPECT_TRUE(result.value());
}

TEST_F(VfsModuleTest, ExistsForNonExistent)
{
    auto fs = fat12_->GetFilesystem();
    vfs::Mount(vfs::Path("/"), {}, fs);

    auto result = vfs::Exists(vfs::Path("/NOTHERE"));
    EXPECT_TRUE(result.has_value());
    EXPECT_FALSE(result.value());
}

TEST_F(VfsModuleTest, MoveFileViaVfs)
{
    auto fs = fat12_->GetFilesystem();
    vfs::Mount(vfs::Path("/"), {}, fs);

    vfs::Path old_path("/OLD.TXT");
    vfs::Path new_path("/NEW.TXT");

    vfs::CreateFile(old_path);

    auto move_result = vfs::Move(old_path, new_path);
    EXPECT_TRUE(move_result.has_value());

    auto old_exists = vfs::Exists(old_path);
    EXPECT_TRUE(old_exists.has_value());
    EXPECT_FALSE(old_exists.value());

    auto new_exists = vfs::Exists(new_path);
    EXPECT_TRUE(new_exists.has_value());
    EXPECT_TRUE(new_exists.value());
}

// =============================================================================
// Read-Only Mount Tests
// =============================================================================

TEST_F(VfsModuleTest, ReadOnlyMountBlocksWrite)
{
    auto fs = fat12_->GetFilesystem();
    vfs::MountOptions opts{.read_only = true};
    vfs::Mount(vfs::Path("/"), opts, fs);

    auto result = vfs::CreateFile(vfs::Path("/TEST.TXT"));
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(vfs::VfsError::kReadOnly, result.error());
}

TEST_F(VfsModuleTest, ReadOnlyMountAllowsRead)
{
    // First mount as writable to create a file
    auto fs = fat12_->GetFilesystem();
    vfs::Mount(vfs::Path("/"), {}, fs);
    vfs::CreateFile(vfs::Path("/READ.TXT"));
    vfs::WriteFile(vfs::Path("/READ.TXT"), "data", 4);
    vfs::Unmount(vfs::Path("/"));

    // Now mount as read-only
    vfs::MountOptions opts{.read_only = true};
    vfs::Mount(vfs::Path("/"), opts, fs);

    char buffer[10] = {0};
    auto result     = vfs::ReadFile(vfs::Path("/READ.TXT"), buffer, 4);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(4u, result.value());
}

// =============================================================================
// Error Handling Tests
// =============================================================================

TEST_F(VfsModuleTest, OperationFailsWithoutMount)
{
    auto result = vfs::CreateFile(vfs::Path("/TEST.TXT"));
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(vfs::VfsError::kMountPointNotFound, result.error());
}

TEST_F(VfsModuleTest, ReadNonExistentFile)
{
    auto fs = fat12_->GetFilesystem();
    vfs::Mount(vfs::Path("/"), {}, fs);

    char buffer[10];
    auto result = vfs::ReadFile(vfs::Path("/NOFILE.TXT"), buffer, 10);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(vfs::VfsError::kFileNotFound, result.error());
}

// =============================================================================
// Complex Scenario Tests
// =============================================================================

TEST_F(VfsModuleTest, CreateAndListMultipleFilesAndDirectories)
{
    auto fs = fat12_->GetFilesystem();
    vfs::Mount(vfs::Path("/"), {}, fs);

    vfs::CreateFile(vfs::Path("/FILE1.TXT"));
    vfs::CreateFile(vfs::Path("/FILE2.TXT"));
    vfs::CreateDirectory(vfs::Path("/DIR1"));
    vfs::CreateDirectory(vfs::Path("/DIR2"));

    int file_count = 0;
    int dir_count  = 0;

    vfs::ListDirectory(vfs::Path("/"), [&](const char *, bool is_dir) {
        if (is_dir) {
            ++dir_count;
        } else {
            ++file_count;
        }
    });

    EXPECT_EQ(2, file_count);
    EXPECT_EQ(2, dir_count);
}

TEST_F(VfsModuleTest, MoveDirectoryWithContents)
{
    auto fs = fat12_->GetFilesystem();
    vfs::Mount(vfs::Path("/"), {}, fs);

    vfs::CreateDirectory(vfs::Path("/OLDDIR"));
    vfs::CreateFile(vfs::Path("/OLDDIR/FILE.TXT"));

    // Write some data to the file
    vfs::WriteFile(vfs::Path("/OLDDIR/FILE.TXT"), "Sample Data", 11);

    auto move_result = vfs::Move(vfs::Path("/OLDDIR"), vfs::Path("/NEWDIR"));
    EXPECT_TRUE(move_result.has_value());

    auto old_exists = vfs::Exists(vfs::Path("/OLDDIR"));
    EXPECT_TRUE(old_exists.has_value());
    EXPECT_FALSE(old_exists.value());

    auto new_exists = vfs::Exists(vfs::Path("/NEWDIR"));
    EXPECT_TRUE(new_exists.has_value());
    EXPECT_TRUE(new_exists.value());

    auto file_exists = vfs::Exists(vfs::Path("/NEWDIR/FILE.TXT"));
    EXPECT_TRUE(file_exists.has_value());
    EXPECT_TRUE(file_exists.value());

    char buffer[20]  = {0};
    auto read_result = vfs::ReadFile(vfs::Path("/NEWDIR/FILE.TXT"), buffer, sizeof(buffer));
    EXPECT_TRUE(read_result.has_value());
    EXPECT_EQ(11u, read_result.value());
    EXPECT_STREQ("Sample Data", buffer);
}

TEST_F(VfsModuleTest, DeleteFileThenDirectory)
{
    auto fs = fat12_->GetFilesystem();
    vfs::Mount(vfs::Path("/"), {}, fs);

    vfs::CreateDirectory(vfs::Path("/DIR"));
    vfs::CreateFile(vfs::Path("/DIR/FILE.TXT"));

    auto delete_file_result = vfs::DeleteFile(vfs::Path("/DIR/FILE.TXT"));
    EXPECT_TRUE(delete_file_result.has_value());

    auto delete_dir_result = vfs::RemoveDirectory(vfs::Path("/DIR"));
    EXPECT_TRUE(delete_dir_result.has_value());

    auto dir_exists = vfs::DirectoryExists(vfs::Path("/DIR"));
    EXPECT_TRUE(dir_exists.has_value());
    EXPECT_FALSE(dir_exists.value());
}

TEST_F(VfsModuleTest, MountMultipleFilesystemsAtDifferentPaths)
{
    auto fs1 = fat12_->GetFilesystem();
    vfs::Mount(vfs::Path("/mnt/fs1"), {}, fs1);

    auto fs2 = fat12_->GetFilesystem();
    vfs::Mount(vfs::Path("/mnt/fs2"), {}, fs2);

    vfs::CreateFile(vfs::Path("/mnt/fs1/FILE1.TXT"));
    vfs::CreateFile(vfs::Path("/mnt/fs2/FILE2.TXT"));

    auto exists1 = vfs::Exists(vfs::Path("/mnt/fs1/FILE1.TXT"));
    EXPECT_TRUE(exists1.has_value());
    EXPECT_TRUE(exists1.value());

    auto exists2 = vfs::Exists(vfs::Path("/mnt/fs1/FILE2.TXT"));
    EXPECT_TRUE(exists2.has_value());
    EXPECT_TRUE(exists2.value());

    auto exists3 = vfs::Exists(vfs::Path("/mnt/fs2/FILE1.TXT"));
    EXPECT_TRUE(exists3.has_value());
    EXPECT_TRUE(exists3.value());

    auto exists4 = vfs::Exists(vfs::Path("/mnt/fs2/FILE2.TXT"));
    EXPECT_TRUE(exists4.has_value());
    EXPECT_TRUE(exists4.value());
}
