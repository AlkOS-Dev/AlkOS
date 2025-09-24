#include "hal/mem/pmm/buddy.hpp"
#include "hal/mem/pmm/bitmap.hpp"
#include "hal/mem/pmm/buddy_config.hpp"
#include "hal/mem/vmm/impl.hpp"

using namespace arch;

using BitmapVmm = internal::VirtualMemoryManager<BitmapPmm>;

void BuddyPmm::InitImpl()
{
    const BuddyPmmConfig &c = GetConfig();

    BitmapPmm bootstrap_pmm;
    bootstrap_pmm.Configure(c.bitmap_pmm_config);
    bootstrap_pmm.Init();

    BitmapVmm bootstrap_vmm(bootstrap_pmm);
    bootstrap_vmm.Configure(c.vmm_config);
    bootstrap_vmm.Init();

    // TODO: Logic to init metadata of buddy allocator
}
