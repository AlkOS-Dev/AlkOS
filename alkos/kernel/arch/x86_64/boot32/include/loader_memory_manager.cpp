#include "loader_memory_manager.hpp"
#include "memory.h"

LoaderMemoryManager::LoaderMemoryManager()
{
    num_pml_tables_stored_ = 1;  ///< The first PML table is the PML4 table
    memset(buffer_, 0, sizeof(buffer_));
}
