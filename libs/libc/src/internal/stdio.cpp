#include "stdio.hpp"

// Fixed buffers for standard streams
byte _stdin_buffer[BUFSIZ];
byte _stdout_buffer[BUFSIZ];
byte _stderr_buffer[BUFSIZ];

FILE _stdin;
FILE _stdout;
FILE _stderr;

FILE *stdin  = &_stdin;
FILE *stdout = &_stdout;
FILE *stderr = &_stderr;

// FILE allocator pool
static FileNode file_pool[FILE_POOL_SIZE];
static FileNode *free_list        = nullptr;
static bool allocator_initialized = false;

/**
 * @brief Initialize the FILE allocator
 */
static void InitFileAllocator()
{
    if (allocator_initialized) {
        return;
    }

    // Build the free list
    for (size_t i = 0; i < FILE_POOL_SIZE - 1; i++) {
        file_pool[i].next = &file_pool[i + 1];
    }
    file_pool[FILE_POOL_SIZE - 1].next = nullptr;

    free_list             = &file_pool[0];
    allocator_initialized = true;
}

FILE *AllocFile()
{
    if (!allocator_initialized) {
        InitFileAllocator();
    }

    if (free_list == nullptr) {
        // Pool exhausted
        return nullptr;
    }

    // Pop from free list
    FileNode *node = free_list;
    free_list      = node->next;

    // Clear the FILE structure
    memset(&node->file, 0, sizeof(FILE));

    return &node->file;
}

void FreeFile(FILE *file)
{
    if (file == nullptr) {
        return;
    }

    // Ensure we only free files from our pool
    if (file < &file_pool[0].file || file > &file_pool[FILE_POOL_SIZE - 1].file) {
        // Not from our pool (might be stdin/stdout/stderr or invalid)
        return;
    }

    // Find the containing FileNode
    // Since FILE is the first member of FileNode, we can cast directly
    FileNode *node = reinterpret_cast<FileNode *>(file);

    // Push back to free list
    node->next = free_list;
    free_list  = node;
}
