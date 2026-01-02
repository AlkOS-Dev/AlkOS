#include <test_module/test.hpp>

#include <string.h>
#include <fs/vfs/fat/fat12.hpp>
#include <fs/vfs/io/in_memory.hpp>
#include <fs/vfs/path.hpp>

// FAT12 disk image layout helper
// A minimal FAT12 filesystem for testing
class Fat12TestHelper
{
    public:
    static constexpr size_t kSectorSize        = 512;
    static constexpr size_t kSectorsPerCluster = 1;
    static constexpr size_t kReservedSectors   = 1;
    static constexpr size_t kNumberOfFats      = 2;
    static constexpr size_t kRootDirEntries    = 16;
    static constexpr size_t kFatSizeSectors    = 1;
    static constexpr size_t kTotalSectors      = 64;  // Small test image

    // Calculate regions
    static constexpr size_t kFatRegionStart = kReservedSectors;
    static constexpr size_t kRootDirStart   = kReservedSectors + kNumberOfFats * kFatSizeSectors;
    static constexpr size_t kRootDirSectors =
        (kRootDirEntries * 32 + kSectorSize - 1) / kSectorSize;
    static constexpr size_t kDataRegionStart = kRootDirStart + kRootDirSectors;

    static constexpr size_t kImageSize = kTotalSectors * kSectorSize;

    static void CreateBootSector(byte *image)
    {
        memset(image, 0, kSectorSize);

        // Jump instruction
        image[0] = 0xEB;
        image[1] = 0x3C;
        image[2] = 0x90;

        // OEM name
        memcpy(image + 3, "MSDOS5.0", 8);

        // Bytes per sector
        *reinterpret_cast<u16 *>(image + 11) = kSectorSize;

        // Sectors per cluster
        image[13] = kSectorsPerCluster;

        // Reserved sectors
        *reinterpret_cast<u16 *>(image + 14) = kReservedSectors;

        // Number of FATs
        image[16] = kNumberOfFats;

        // Root directory entries
        *reinterpret_cast<u16 *>(image + 17) = kRootDirEntries;

        // Total sectors (small volume)
        *reinterpret_cast<u16 *>(image + 19) = kTotalSectors;

        // Media descriptor (fixed disk)
        image[21] = 0xF8;

        // FAT size in sectors
        *reinterpret_cast<u16 *>(image + 22) = kFatSizeSectors;

        // Sectors per track
        *reinterpret_cast<u16 *>(image + 24) = 18;

        // Number of heads
        *reinterpret_cast<u16 *>(image + 26) = 2;

        // Hidden sectors
        *reinterpret_cast<u32 *>(image + 28) = 0;

        // Total sectors (dword, for large volumes)
        *reinterpret_cast<u32 *>(image + 32) = 0;

        // Drive number
        image[36] = 0x80;

        // Boot signature
        image[38] = 0x29;

        // Volume ID
        *reinterpret_cast<u32 *>(image + 39) = 0x12345678;

        // Volume label
        memcpy(image + 43, "TEST VOLUME", 11);

        // Filesystem type
        memcpy(image + 54, "FAT12   ", 8);

        // Boot signature at end
        image[510] = 0x55;
        image[511] = 0xAA;
    }

    static void CreateFatTables(byte *image)
    {
        // First FAT
        byte *fat1 = image + kFatRegionStart * kSectorSize;
        memset(fat1, 0, kFatSizeSectors * kSectorSize);

        // FAT[0] = media descriptor | 0xF00
        fat1[0] = 0xF8;  // Media descriptor
        fat1[1] = 0xFF;
        fat1[2] = 0xFF;

        // Second FAT (copy)
        byte *fat2 = image + (kFatRegionStart + kFatSizeSectors) * kSectorSize;
        memcpy(fat2, fat1, kFatSizeSectors * kSectorSize);
    }

    static void CreateRootDirectory(byte *image)
    {
        byte *root_dir = image + kRootDirStart * kSectorSize;
        memset(root_dir, 0, kRootDirSectors * kSectorSize);

        // Create volume label entry
        memcpy(root_dir, "TEST VOLUME", 11);
        root_dir[11] = 0x08;  // Volume label attribute
    }

    static void CreateMinimalImage(byte *image)
    {
        memset(image, 0, kImageSize);
        CreateBootSector(image);
        CreateFatTables(image);
        CreateRootDirectory(image);
    }
};

class Fat12Test : public TestGroupBase
{
    protected:
    alignas(16) byte disk_image[Fat12TestHelper::kImageSize];
    vfs::io::InMemory *io{nullptr};
    vfs::Fat12<vfs::io::InMemory> *fat12{nullptr};

    void Setup_() override
    {
        Fat12TestHelper::CreateMinimalImage(disk_image);

        io = Mem::KMalloc<vfs::io::InMemory>().value_or(nullptr);
        EXPECT_NOT_NULL(io);
        std::construct_at<vfs::io::InMemory>(io, disk_image, Fat12TestHelper::kSectorSize);

        fat12 = Mem::KMalloc<vfs::Fat12<vfs::io::InMemory>>().value_or(nullptr);
        EXPECT_NOT_NULL(fat12);
        std::construct_at<vfs::Fat12<vfs::io::InMemory>>(fat12, *io);
    }

    void TearDown_() override
    {
        std::destroy_at(fat12);
        Mem::KFree(fat12);

        std::destroy_at(io);
        Mem::KFree(io);
    }
};

// Basic validation tests
TEST_F(Fat12Test, IsValidReturnsTrueForValidImage)
{
    vfs::io::InMemory io(disk_image, Fat12TestHelper::kSectorSize);
    EXPECT_TRUE(vfs::Fat12<vfs::io::InMemory>::IsValid(io));
}

TEST_F(Fat12Test, IsValidReturnsFalseForInvalidSignature)
{
    // Corrupt boot signature
    disk_image[510] = 0x00;
    disk_image[511] = 0x00;

    vfs::io::InMemory io(disk_image, Fat12TestHelper::kSectorSize);
    EXPECT_FALSE(vfs::Fat12<vfs::io::InMemory>::IsValid(io));
}

TEST_F(Fat12Test, IsValidReturnsFalseForInvalidJumpInstruction)
{
    // Corrupt jump instruction
    disk_image[0] = 0x00;

    vfs::io::InMemory io(disk_image, Fat12TestHelper::kSectorSize);
    EXPECT_FALSE(vfs::Fat12<vfs::io::InMemory>::IsValid(io));
}

// Directory listing tests
TEST_F(Fat12Test, ListDirectoryRootReturnsVolumeLabel)
{
    auto fs = fat12->GetFilesystem();

    int entry_count = 0;
    vfs::Path root("/");

    fs.ListDirectory(root, [&](const char *, bool) {
        ++entry_count;
    });

    // Volume label should be skipped, so expect 0 entries
    EXPECT_EQ(0, entry_count);
}

// File existence tests
TEST_F(Fat12Test, FileExistsReturnsFalseForNonExistentFile)
{
    auto fs = fat12->GetFilesystem();

    vfs::Path path("/NOFILE.TXT");
    auto result = fs.FileExists(path);

    EXPECT_TRUE(result.has_value());
    EXPECT_FALSE(result.value());
}

TEST_F(Fat12Test, DirectoryExistsReturnsTrueForRoot)
{
    auto fs = fat12->GetFilesystem();

    vfs::Path path("/");
    auto result = fs.DirectoryExists(path);

    EXPECT_TRUE(result.has_value());
    EXPECT_TRUE(result.value());
}

TEST_F(Fat12Test, DirectoryExistsReturnsFalseForNonExistentDir)
{
    auto fs = fat12->GetFilesystem();

    vfs::Path path("/NODIR");
    auto result = fs.DirectoryExists(path);

    EXPECT_TRUE(result.has_value());
    EXPECT_FALSE(result.value());
}

TEST_F(Fat12Test, ExistsReturnsTrueForRoot)
{
    auto fs = fat12->GetFilesystem();

    vfs::Path path("/");
    auto result = fs.Exists(path);

    EXPECT_TRUE(result.has_value());
    EXPECT_TRUE(result.value());
}

// File creation tests
TEST_F(Fat12Test, CreateFileSucceeds)
{
    auto fs = fat12->GetFilesystem();

    vfs::Path path("/TEST.TXT");
    auto result = fs.CreateFile(path);

    EXPECT_TRUE(result.has_value());
}

TEST_F(Fat12Test, CreateFileFailsForInvalidName)
{
    auto fs = fat12->GetFilesystem();

    // Name too long for 8.3 format
    vfs::Path path("/VERYLONGFILENAME.TXT");
    auto result = fs.CreateFile(path);

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), vfs::VfsError::kInvalidName);
}

TEST_F(Fat12Test, CreateFileThenFileExists)
{
    auto fs = fat12->GetFilesystem();

    vfs::Path path("/NEWFILE.TXT");

    // Create file
    auto create_result = fs.CreateFile(path);
    EXPECT_TRUE(create_result.has_value());

    // Verify it exists
    auto exists_result = fs.FileExists(path);
    EXPECT_TRUE(exists_result.has_value());
    EXPECT_TRUE(exists_result.value());
}

TEST_F(Fat12Test, CreateFileFailsIfAlreadyExists)
{
    auto fs = fat12->GetFilesystem();

    vfs::Path path("/DUP.TXT");

    // Create file first time
    auto result1 = fs.CreateFile(path);
    EXPECT_TRUE(result1.has_value());

    // Try to create again
    auto result2 = fs.CreateFile(path);
    EXPECT_FALSE(result2.has_value());
    EXPECT_EQ(result2.error(), vfs::VfsError::kAlreadyExists);
}

// Directory creation tests
TEST_F(Fat12Test, CreateDirectorySucceeds)
{
    auto fs = fat12->GetFilesystem();

    vfs::Path path("/TESTDIR");
    auto result = fs.CreateDirectory(path);

    EXPECT_TRUE(result.has_value());
}

TEST_F(Fat12Test, CreateDirectoryThenExists)
{
    auto fs = fat12->GetFilesystem();

    vfs::Path path("/MYDIR");

    // Create directory
    auto create_result = fs.CreateDirectory(path);
    EXPECT_TRUE(create_result.has_value());

    // Verify it exists
    auto exists_result = fs.DirectoryExists(path);
    EXPECT_TRUE(exists_result.has_value());
    EXPECT_TRUE(exists_result.value());
}

// File deletion tests
TEST_F(Fat12Test, DeleteFileSucceeds)
{
    auto fs = fat12->GetFilesystem();

    vfs::Path path("/TODEL.TXT");

    // Create file first
    auto create_result = fs.CreateFile(path);
    EXPECT_TRUE(create_result.has_value());

    // Delete it
    auto delete_result = fs.DeleteFile(path);
    EXPECT_TRUE(delete_result.has_value());

    // Verify it's gone
    auto exists_result = fs.FileExists(path);
    EXPECT_TRUE(exists_result.has_value());
    EXPECT_FALSE(exists_result.value());
}

TEST_F(Fat12Test, DeleteFileFailsForNonExistent)
{
    auto fs = fat12->GetFilesystem();

    vfs::Path path("/NOFILE.TXT");
    auto result = fs.DeleteFile(path);

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), vfs::VfsError::kFileNotFound);
}

// File read/write tests
TEST_F(Fat12Test, WriteAndReadFile)
{
    auto fs = fat12->GetFilesystem();

    vfs::Path path("/DATA.TXT");

    // Create file
    auto create_result = fs.CreateFile(path);
    EXPECT_TRUE(create_result.has_value());

    // Write data
    const char *test_data = "Hello, FAT12!";
    size_t data_len       = strlen(test_data);

    auto write_result = fs.WriteFile(path, test_data, data_len, 0);
    EXPECT_TRUE(write_result.has_value());
    EXPECT_EQ(write_result.value(), data_len);

    // Read it back
    char read_buffer[64] = {0};
    auto read_result     = fs.ReadFile(path, read_buffer, data_len, 0);
    EXPECT_TRUE(read_result.has_value());
    EXPECT_EQ(read_result.value(), data_len);

    // Verify content
    EXPECT_EQ(memcmp(read_buffer, test_data, data_len), 0);
}

TEST_F(Fat12Test, GetFileSizeAfterWrite)
{
    auto fs = fat12->GetFilesystem();

    vfs::Path path("/SIZE.TXT");

    // Create and write
    fs.CreateFile(path);
    const char *data = "Test content";
    size_t data_len  = strlen(data);
    fs.WriteFile(path, data, data_len, 0);

    // Check size
    auto size_result = fs.GetFileSize(path);
    EXPECT_TRUE(size_result.has_value());
    EXPECT_EQ(size_result.value(), data_len);
}

TEST_F(Fat12Test, ReadFileFailsForNonExistent)
{
    auto fs = fat12->GetFilesystem();

    vfs::Path path("/NOFILE.TXT");
    char buffer[64];

    auto result = fs.ReadFile(path, buffer, sizeof(buffer), 0);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), vfs::VfsError::kFileNotFound);
}

// Remove directory tests
TEST_F(Fat12Test, RemoveEmptyDirectorySucceeds)
{
    auto fs = fat12->GetFilesystem();

    vfs::Path path("/EMPTYDIR");

    // Create directory
    auto create_result = fs.CreateDirectory(path);
    EXPECT_TRUE(create_result.has_value());

    // Remove it
    auto remove_result = fs.RemoveDirectory(path);
    EXPECT_TRUE(remove_result.has_value());

    // Verify it's gone
    auto exists_result = fs.DirectoryExists(path);
    EXPECT_TRUE(exists_result.has_value());
    EXPECT_FALSE(exists_result.value());
}

TEST_F(Fat12Test, RemoveDirectoryFailsForNonExistent)
{
    auto fs = fat12->GetFilesystem();

    vfs::Path path("/NODIR");
    auto result = fs.RemoveDirectory(path);

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), vfs::VfsError::kDirectoryNotFound);
}

// Move tests
TEST_F(Fat12Test, MoveFileSucceeds)
{
    auto fs = fat12->GetFilesystem();

    vfs::Path old_path("/OLD.TXT");
    vfs::Path new_path("/NEW.TXT");

    // Create file
    fs.CreateFile(old_path);

    // Move it
    auto move_result = fs.Move(old_path, new_path);
    EXPECT_TRUE(move_result.has_value());

    // Verify old doesn't exist
    auto old_exists = fs.FileExists(old_path);
    EXPECT_TRUE(old_exists.has_value());
    EXPECT_FALSE(old_exists.value());

    // Verify new exists
    auto new_exists = fs.FileExists(new_path);
    EXPECT_TRUE(new_exists.has_value());
    EXPECT_TRUE(new_exists.value());
}

TEST_F(Fat12Test, MoveFailsIfDestinationExists)
{
    auto fs = fat12->GetFilesystem();

    vfs::Path path1("/FILE1.TXT");
    vfs::Path path2("/FILE2.TXT");

    // Create both files
    fs.CreateFile(path1);
    fs.CreateFile(path2);

    // Try to move
    auto result = fs.Move(path1, path2);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), vfs::VfsError::kAlreadyExists);
}
