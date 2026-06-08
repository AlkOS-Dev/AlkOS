#include <string.h>

#include <alkos/sys/proc.h>

#include "assert.h"
#include "stdlib.h"

// Simple malloc implementation
// https://gist.github.com/mshr-h/9636fa0adcf834103b1b

namespace
{
int has_initialized = 0;
void *managed_memory_start;
void *last_valid_address;

struct mem_control_block {
    int is_available;
    int size;
};

void malloc_init()
{
    ASSERT_FALSE(kIsKernel);
    /* grab the last valid address from the OS */
    last_valid_address = GetHeapStart();
    /* we don't have any memory to manage yet, so
     * just set the beginning to be last_valid_address */
    managed_memory_start = last_valid_address;
    /* Okay, we're initialized and ready to go */
    has_initialized = 1;
}
}  // namespace

void *malloc(size_t numbytes)
{
    /* Holds where we are looking in memory */
    /* This is the same as current_location, but cast to a memory_control_block */
    struct mem_control_block *current_location_mcb;
    /* This is the memory location we will return.
       It will be set to 0 until we find something suitable */

    /* Initialize if we haven't already done so */
    if (!has_initialized) {
        malloc_init();
    }

    /* The memory we search for has to include the memory
     * control block, but the user of malloc doesn't need
     * to know this, so we'll just add it in for them. */
    numbytes = numbytes + sizeof(mem_control_block);

    /* Set memory_location to 0 until we find a suitable location */
    void *memory_location = nullptr;

    /* Begin searching at the start of managed memory */
    void *current_location = managed_memory_start;

    /* Keep going until we have searched all allocated space */
    while (current_location != last_valid_address) {
        current_location_mcb = static_cast<mem_control_block *>(current_location);
        if (current_location_mcb->is_available) {
            if (current_location_mcb->size >= static_cast<int>(numbytes)) {
                /* Woohoo! We've found an open, appropriately-size location. */
                current_location_mcb->is_available = 0;
                memory_location                    = current_location;
                break;
            }
        }
        /* If we made it here, it's because the current memory
         * block not suitable, move to the next one */
        current_location = static_cast<char *>(current_location) + current_location_mcb->size;
    }

    /* If we still don't have a valid location, abort */
    if (!memory_location) {
        /* The new memory will be where the last valid * address left off */
        memory_location = last_valid_address;
        /* We'll move the last valid address forward * numbytes */
        last_valid_address = last_valid_address + numbytes;
        /* We need to initialize the mem_control_block */
        current_location_mcb               = static_cast<mem_control_block *>(memory_location);
        current_location_mcb->is_available = 0;
        current_location_mcb->size         = numbytes;
    }

    /* Move the pointer past the mem_control_block */
    memory_location = static_cast<char *>(memory_location) + sizeof(mem_control_block);
    return memory_location;
}

void free(void *firstbyte)
{
    if (!firstbyte)
        return;

    mem_control_block *mcb = reinterpret_cast<mem_control_block *>(
        static_cast<char *>(firstbyte) - sizeof(struct mem_control_block)
    );
    mcb->is_available = 1;
}

void *calloc(size_t nmemb, size_t size)
{
    size_t total_size = nmemb * size;
    void *ptr         = malloc(total_size);
    if (ptr) {
        memset(ptr, 0, total_size);
    }
    return ptr;
}

void *realloc(void *ptr, size_t size)
{
    if (!ptr) {
        return malloc(size);
    }
    if (size == 0) {
        free(ptr);
        return nullptr;
    }

    void *new_ptr = malloc(size);
    if (new_ptr) {
        // Copy old data to new location
        const mem_control_block *mcb = reinterpret_cast<mem_control_block *>(
            static_cast<char *>(ptr) - sizeof(struct mem_control_block)
        );
        const size_t old_size  = mcb->size - sizeof(struct mem_control_block);
        const size_t copy_size = (old_size < size) ? old_size : size;

        memcpy(new_ptr, ptr, copy_size);
        free(ptr);
    }
    return new_ptr;
}
