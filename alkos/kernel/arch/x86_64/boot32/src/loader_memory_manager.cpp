#include "loader_memory_manager.hpp"
#include "assert.h"
#include "debug.hpp"
#include "extensions/new.hpp"
#include "memory.h"

// Note: The alignment here is a strict requirement for the PML tables and if the
// initial object is not aligned, the PML tables will not be aligned either.
byte kLoaderPreAllocatedMemory[sizeof(LoaderMemoryManager)] __attribute__((aligned(4096)));

LoaderMemoryManager::LoaderMemoryManager()
{
    num_pml_tables_stored_ = 1;  ///< The first PML table is the PML4 table
    memset(buffer_, 0, sizeof(buffer_));
}
LoaderMemoryManager::PML4_t *LoaderMemoryManager::GetPml4Table() { return &buffer_[kPml4Index]; }
