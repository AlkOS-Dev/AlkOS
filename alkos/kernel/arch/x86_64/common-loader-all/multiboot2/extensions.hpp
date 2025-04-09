#ifndef ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MULTIBOOT2_EXTENSIONS_HPP_
#define ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MULTIBOOT2_EXTENSIONS_HPP_

#include <extensions/bit.hpp>
#include <extensions/concepts.hpp>
#include <extensions/tuple.hpp>
#include <extensions/type_traits.hpp>
#include <extensions/types.hpp>
#include "multiboot2/concepts.hpp"
#include "multiboot2/multiboot2.h"
#include "todo.hpp"

namespace multiboot
{

static constexpr u32 kInvalidTagNumber = kFullMask<u32>;

// Primary template defaults to 0. This can't be enforced with a concept because
// tags do not have a common base class. (framebuffer_t)
template <class TagType>
struct TagNumber {
    static constexpr u32 value            = kInvalidTagNumber;
    static constexpr const char* kTagName = "INVALID_TAG";
};

// Macro to conveniently specialize TagNumber for a given tag structure and its constant.
#define DEFINE_TAG_NUMBER(TagStruct, TagValue)              \
    template <>                                             \
    struct TagNumber<TagStruct> {                           \
        static constexpr u32 kInvalidTagNumber = ~0u;       \
        static constexpr u32 value             = TagValue;  \
        static constexpr const char* kTagName  = #TagValue; \
    }

// Specializations for each tag type based on the MULTIBOOT_TAG_TYPE_ constants.
DEFINE_TAG_NUMBER(tag_string_t, kMultibootTagTypeCmdline);
DEFINE_TAG_NUMBER(tag_module_t, kMultibootTagTypeModule);
DEFINE_TAG_NUMBER(tag_basic_meminfo_t, kMultibootTagTypeBasicMeminfo);
DEFINE_TAG_NUMBER(tag_bootdev_t, kMultibootTagTypeBootdev);
DEFINE_TAG_NUMBER(tag_mmap_t, kMultibootTagTypeMmap);
DEFINE_TAG_NUMBER(tag_vbe_t, kMultibootTagTypeVbe);
DEFINE_TAG_NUMBER(tag_framebuffer_t, kMultibootTagTypeFramebuffer);
DEFINE_TAG_NUMBER(tag_elf_sections_t, kMultibootTagTypeElfSections);
DEFINE_TAG_NUMBER(tag_apm_t, kMultibootTagTypeApm);
DEFINE_TAG_NUMBER(tag_efi32_t, kMultibootTagTypeEfi32);
DEFINE_TAG_NUMBER(tag_efi64_t, kMultibootTagTypeEfi64);
DEFINE_TAG_NUMBER(tag_smbios_t, kMultibootTagTypeSmbios);
DEFINE_TAG_NUMBER(tag_old_acpi_t, kMultibootTagTypeAcpiOld);
DEFINE_TAG_NUMBER(tag_new_acpi_t, kMultibootTagTypeAcpiNew);
DEFINE_TAG_NUMBER(tag_network_t, kMultibootTagTypeNetwork);
DEFINE_TAG_NUMBER(tag_efi_mmap_t, kMultibootTagTypeEfiMmap);
DEFINE_TAG_NUMBER(tag_efi32_ih_t, kMultibootTagTypeEfi32Ih);
DEFINE_TAG_NUMBER(tag_efi64_ih_t, kMultibootTagTypeEfi64Ih);
DEFINE_TAG_NUMBER(tag_load_base_addr_t, kMultibootTagTypeLoadBaseAddr);

/**
 * @brief Find a tag in the multiboot info.
 *
 * @param multiboot_info_addr The address of the multiboot info.
 */
template <
    class Tag,
    TagFilter<Tag> auto Filter = [](Tag*) constexpr -> bool {
        return true;
    }>
Tag* FindTagInMultibootInfo(void* multiboot_info_addr);

/**
 * @brief Walk the memory map.
 *
 * @param mmap_tag The memory map tag pointer.
 * @param callback The callback function that accepts a memory map entry.
 */
template <MemoryMapCallback Callback>
void WalkMemoryMap(tag_mmap_t* mmap_tag, Callback callback);

std::tuple<u64, u64> GetMultibootStructureBounds(void* multiboot_info_addr);

}  // namespace multiboot

#include "extensions.tpp"

#endif  // ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MULTIBOOT2_EXTENSIONS_HPP_
