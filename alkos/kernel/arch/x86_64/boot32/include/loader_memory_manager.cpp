#include "loader_memory_manager.hpp"
#include "assert.h"
#include "memory.h"

LoaderMemoryManager::LoaderMemoryManager()
{
    num_pml_tables_stored_ = 1;  ///< The first PML table is the PML4 table
    memset(buffer_, 0, sizeof(buffer_));
}
LoaderMemoryManager::PML4_t *LoaderMemoryManager::GetPml4Table() { return &buffer_[kPml4Index]; }
