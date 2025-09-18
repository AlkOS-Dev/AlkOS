#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif

#if !defined(__i386__)
#error "loader_32 needs to be compiled with a i386-elf compiler"
#endif

#include <extensions/algorithm.hpp>
#include <extensions/debug.hpp>
#include <extensions/defines.hpp>
#include <extensions/internal/formats.hpp>
#include <extensions/new.hpp>

#include "abi/transition_data.hpp"
#include "cpu/utils.hpp"
#include "elf/elf_64.hpp"
#include "elf/elf_dynamic.hpp"
#include "elf/error.hpp"
#include "mem/memory_manager.hpp"
#include "mem/pmm.hpp"
#include "mem/vmm.hpp"
#include "multiboot2/info.hpp"
#include "multiboot2/memory_map.hpp"
#include "multiboot2/multiboot2.h"
#include "settings.hpp"
#include "sys/panic.hpp"
#include "sys/terminal.hpp"

using namespace Multiboot;

//==============================================================================
// External Functions and Variables
//==============================================================================

BEGIN_DECL_C
int CheckCpuId();
int CheckLongMode();
void EnablePaging(void* pml4_table_address);
void EnableLongMode();
void EnterElf64(
    void* higher_32_bits_of_entry_address, void* lower_32_bits_of_entry_address,
    void* transition_data_address
);

// Defined in .ld
extern const char multiboot_header_start[];
extern const char multiboot_header_end[];
extern const char loader_32_start[];
extern const char loader_32_end[];

END_DECL_C

//==============================================================================
// Global Definitions
//==============================================================================

struct MemoryManagers {
    PhysicalMemoryManager& pmm;
    VirtualMemoryManager& vmm;
};

//==============================================================================
// Global Data
//==============================================================================

alignas(64) static TransitionData transition_data;
alignas(
    PageSize<PageSizeTag::k4Kb>()
) static byte kPmmPreAllocatedMemory[sizeof(PhysicalMemoryManager)];
alignas(
    PageSize<PageSizeTag::k4Kb>()
) static byte kVmmPreAllocatedMemory[sizeof(VirtualMemoryManager)];

u32 mb_hdr_start_addr;
u32 mb_hdr_end_addr;

u32 ld_start_addr;
u32 ld_end_addr;

//==============================================================================
// High-Level Boot Steps
//==============================================================================

static void InitializeAndVerifyEnvironment(u32 boot_loader_magic)
{
    TerminalInit();
    TRACE_INFO("Loader 32-Bit Stage");

    if (boot_loader_magic != Multiboot::kMultiboot2BootloaderMagic) {
        KernelPanic("Multiboot2 check failed! Invalid magic number.");
    }

    TRACE_INFO("Checking for required hardware features...");
    if (CheckCpuId()) {
        KernelPanic("CPUID check failed!");
    }
    if (CheckLongMode()) {
        KernelPanic("Long mode check failed!");
    }

    BlockHardwareInterrupts();

    ld_start_addr = reinterpret_cast<u32>(loader_32_start);
    ld_end_addr   = reinterpret_cast<u32>(loader_32_end);

    mb_hdr_start_addr = reinterpret_cast<u32>(multiboot_header_start);
    mb_hdr_end_addr   = reinterpret_cast<u32>(multiboot_header_end);
}

MemoryManagers SetupMemoryManagement(MultibootInfo& multiboot_info)
{
    TRACE_DEBUG("Setting up memory management...");

    // Add available memory regions from the Multiboot map
    auto mmap_tag_res = multiboot_info.FindTag<TagMmap>();
    R_ASSERT_TRUE(mmap_tag_res, "Failed to find memory map tag in multiboot info");

    u64 lowest_safe_addr =
        std::max(static_cast<u64>(mb_hdr_end_addr), static_cast<u64>(ld_end_addr));

    const MemoryMap mmap(*mmap_tag_res);
    auto pmm_res = PhysicalMemoryManager::Create(mmap, lowest_safe_addr, kPmmPreAllocatedMemory);
    R_ASSERT_TRUE(pmm_res, "Physical memory manager creation failed");

    TRACE_DEBUG("Creating Physical Memory Manager...");
    auto* pmm_ptr = *pmm_res;
    auto& pmm     = *pmm_ptr;

    TRACE_DEBUG("Creating Virtual Memory Manager...");
    auto* vmm_ptr = new (kVmmPreAllocatedMemory) VirtualMemoryManager(pmm);
    auto& vmm     = *vmm_ptr;

    TRACE_DEBUG("Marking used memory as reserved");
    const u64 ld_span    = static_cast<u64>(ld_end_addr) - ld_start_addr;
    const u32 al_ld_addr = AlignDown(ld_start_addr, PageSize<PageSizeTag::k4Kb>());
    const u64 al_ld_span = AlignUp(ld_span, PageSize<PageSizeTag::k4Kb>());
    pmm.Reserve(PhysicalPtr<void>(al_ld_addr), al_ld_span);

    const u64 mb_hdr_span    = static_cast<u64>(mb_hdr_end_addr) - mb_hdr_start_addr;
    const u32 al_mb_hdr_addr = AlignDown(mb_hdr_start_addr, PageSize<PageSizeTag::k4Kb>());
    const u64 al_mb_hdr_span = AlignUp(al_mb_hdr_span, PageSize<PageSizeTag::k4Kb>());
    pmm.Reserve(PhysicalPtr<void>(al_mb_hdr_addr), al_mb_hdr_span);

    TRACE_DEBUG("Identity mapping memory...");
    vmm.Map<&PhysicalMemoryManager::Alloc32, PageSizeTag::k1Gb>(
        0, 0, 10 * PageSize<PageSizeTag::k1Gb>(), kPresentBit | kWriteBit | kGlobalBit
    );

    vmm.GetPml4Table().ValuePtr();

    return {pmm, vmm};
}

static void EnableCpuFeatures(MemoryManagers mem_managers)
{
    auto& vmm = mem_managers.vmm;
    PhysicalPtr<void> pml4_phys_ptr(vmm.GetPml4Table());

    TRACE_INFO("Enabling long mode and paging...");
    EnableLongMode();
    EnablePaging(pml4_phys_ptr.ValuePtr());
}

static void RelocateNextStageModule(byte* elf_data, u64 load_base_addr)
{
    const auto* header               = reinterpret_cast<const Elf64::Header*>(elf_data);
    const auto* program_header_table = reinterpret_cast<const Elf64::ProgramHeaderEntry*>(
        elf_data + header->program_header_table_file_offset
    );

    const Elf64::ProgramHeaderEntry* dynamic_header = nullptr;
    for (u16 i = 0; i < header->program_header_table_entry_count; i++) {
        if (program_header_table[i].type == 2 /* PT_DYNAMIC */) {
            dynamic_header = &program_header_table[i];
            break;
        }
    }

    if (!dynamic_header) {
        KernelPanic("Could not find PT_DYNAMIC header in next stage module!");
    }

    Elf64::Dyn* dynamic_section = reinterpret_cast<Elf64::Dyn*>(elf_data + dynamic_header->offset);

    u64 rela_addr     = 0;
    u64 rela_size     = 0;
    u64 rela_ent_size = 0;

    for (int i = 0; dynamic_section[i].d_tag != Elf64::DT_NULL; ++i) {
        switch (dynamic_section[i].d_tag) {
            case Elf64::DT_RELA:
                rela_addr = dynamic_section[i].d_un.d_ptr;
                break;
            case Elf64::DT_RELASZ:
                rela_size = dynamic_section[i].d_un.d_val;
                break;
            case Elf64::DT_RELAENT:
                rela_ent_size = dynamic_section[i].d_un.d_val;
                break;
        }
    }

    // The addresses in the dynamic section are virtual addresses. We need to find
    // which segment they belong to in order to find their file offset.
    u64 rela_file_offset = 0;
    for (u16 i = 0; i < header->program_header_table_entry_count; i++) {
        const auto& ph = program_header_table[i];
        if (ph.type == 1 /* PT_LOAD */ && rela_addr >= ph.virtual_address &&
            rela_addr < ph.virtual_address + ph.size_in_file_bytes) {
            rela_file_offset = rela_addr - ph.virtual_address + ph.offset;
            break;
        }
    }

    if (rela_file_offset == 0 || rela_size == 0) {
        KernelPanic("Could not find relocation table in next stage module!");
    }

    auto* rela_table    = reinterpret_cast<Elf64::Rela*>(elf_data + rela_file_offset);
    u64 num_relocations = rela_size / rela_ent_size;

    for (u64 i = 0; i < num_relocations; ++i) {
        auto& reloc = rela_table[i];
        // We only support R_X86_64_RELATIVE for now, which is sufficient for a PIE
        if ((reloc.r_info & 0xFFFFFFFF) == Elf64::R_X86_64_RELATIVE) {
            // The location to patch is at `load_base_addr + r_offset`
            u64* patch_addr = reinterpret_cast<u64*>(load_base_addr + reloc.r_offset);
            // The new value is `load_base_addr + r_addend`
            *patch_addr = load_base_addr + reloc.r_addend;
        }
    }
}

static u64 LoadNextStageModule(MultibootInfo& multiboot_info, MemoryManagers mem_managers)
{
    auto& pmm = mem_managers.pmm;
    auto& vmm = mem_managers.vmm;

    TRACE_DEBUG("Loading next stage module: '%s' ...", kLoader64ModuleCmdline);

    TRACE_DEBUG("Searching for next stage module tag in multiboot info...");
    auto next_module_res = multiboot_info.FindTag<TagModule>([](TagModule* tag) {
        return strcmp(tag->cmdline, kLoader64ModuleCmdline) == 0;
    });
    ASSERT_TRUE(next_module_res, "Failed to find the next stage module tag in multiboot info");
    auto next_module_tag = *next_module_res;

    TRACE_DEBUG("Validating the next stage module as ELF64...");
    auto elf = PhysicalPtr<byte>(next_module_tag->mod_start);
    u64 virt_start, virt_end;
    auto elf_bounds_res = Elf64::GetProgramBounds(elf.ValuePtr(), virt_start, virt_end);
    ASSERT_TRUE(elf_bounds_res, "Failed to get ELF64 program bounds");

    TRACE_DEBUG("Allocating memory for the next stage module...");
    u64 module_size      = virt_end - virt_start;
    auto module_dest_res = pmm.AllocContiguous32(module_size);
    ASSERT_TRUE(module_dest_res, "Failed to allocate memory for the next stage module");
    auto module_dest = PhysicalPtr<byte>(*module_dest_res);

    TRACE_DEBUG("Loading the next stage module as ELF64...");
    auto entry_point_res = Elf64::Load(elf.ValuePtr(), module_dest.Value());
    ASSERT_TRUE(entry_point_res, "Failed to load the next stage module as ELF64");

    TRACE_DEBUG("Relocating the next stage module...");
    RelocateNextStageModule(elf.ValuePtr(), module_dest.Value());

    return entry_point_res.value();
}

NO_RET static void TransitionTo64BitMode(
    u64 entry_point, MemoryManagers mem_managers, u32 multiboot_info_addr_32
)
{
    auto& pmm = mem_managers.pmm;
    auto& vmm = mem_managers.vmm;

    TRACE_INFO("Preparing to transition to 64-bit mode...");

    auto pmm_state = pmm.GetState();
    auto vmm_state = vmm.GetState();

    // Prepare the data to be passed to the 64-bit stage
    transition_data.multiboot_info_addr         = multiboot_info_addr_32;
    transition_data.multiboot_header_start_addr = reinterpret_cast<u64>(multiboot_header_start);
    transition_data.multiboot_header_end_addr   = reinterpret_cast<u64>(multiboot_header_end);

    // Note : Stupid but compiler creates a buggy assigment operator for some reason
    transition_data.pmm_state.total_pages        = pmm_state.total_pages;
    transition_data.pmm_state.bitmap_addr        = pmm_state.bitmap_addr;
    transition_data.pmm_state.iteration_index    = pmm_state.iteration_index;
    transition_data.pmm_state.iteration_index_32 = pmm_state.iteration_index_32;

    transition_data.vmm_state.pml_4_table_phys_addr = vmm_state.pml_4_table_phys_addr;

    // TODO: Update
    TRACE(
        "Transition Data:\n"
        "  multiboot_info_addr:         0x%llX\n"
        "  multiboot_header_start_addr: 0x%llX\n"
        "  multiboot_header_end_addr:   0x%llX\n"
        "  pmm1:                    0x%llX\n"
        "  pmm2:                    0x%llX\n"
        "  vmm1:                    0x%llX\n",
        transition_data.multiboot_info_addr, transition_data.multiboot_header_start_addr,
        transition_data.multiboot_header_end_addr, transition_data.pmm_state.bitmap_addr,
        transition_data.pmm_state.total_pages, transition_data.vmm_state.pml_4_table_phys_addr
    );

    TRACE_DEBUG("Freeing memory of 32-bit loader before jump");
    const u64 ld_span    = static_cast<u64>(ld_end_addr) - ld_start_addr;
    const u32 al_ld_addr = AlignDown(ld_start_addr, PageSize<PageSizeTag::k4Kb>());
    const u64 al_ld_span = AlignUp(ld_span, PageSize<PageSizeTag::k4Kb>());
    pmm.Free(PhysicalPtr<void>(al_ld_addr), al_ld_span);

    void* entry_high = reinterpret_cast<void*>(static_cast<u32>(entry_point >> 32));
    void* entry_low  = reinterpret_cast<void*>(static_cast<u32>(entry_point & kBitMask32));
    TRACE_INFO("Jumping to 64-bit loader at entry point: 0x%llX", entry_point);
    EnterElf64(entry_high, entry_low, &transition_data);

    KernelPanic("EnterElf64 should not return!");
}

//==================================================================================
// Main Entry Point
//==================================================================================

extern "C" void MainLoader32(u32 boot_loader_magic, u32 multiboot_info_addr_32)
{
    InitializeAndVerifyEnvironment(boot_loader_magic);

    MultibootInfo multiboot_info(multiboot_info_addr_32);

    MemoryManagers mem_managers = SetupMemoryManagement(multiboot_info);

    EnableCpuFeatures(mem_managers);

    u64 next_stage_entry_point = LoadNextStageModule(multiboot_info, mem_managers);

    TransitionTo64BitMode(next_stage_entry_point, mem_managers, multiboot_info_addr_32);
}
