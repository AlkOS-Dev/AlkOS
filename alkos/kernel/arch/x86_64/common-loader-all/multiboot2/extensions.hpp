#ifndef ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MULTIBOOT2_EXTENSIONS_HPP_
#define ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MULTIBOOT2_EXTENSIONS_HPP_

#include <multiboot2/multiboot2.h>
#include <extensions/concepts.hpp>
#include <extensions/type_traits.hpp>
#include <extensions/types.hpp>
#include <todo.hpp>

namespace multiboot
{

/// Callback that returns true if the tag is the one we are looking for and false otherwise
template <class FilterT, class TagT>
concept TagFilter = requires(FilterT filter, TagT* tag) {
    { filter(tag) } -> std::convertible_to<bool>;
};

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
DEFINE_TAG_NUMBER(tag_string_t, MULTIBOOT_TAG_TYPE_CMDLINE);
DEFINE_TAG_NUMBER(tag_module_t, MULTIBOOT_TAG_TYPE_MODULE);
DEFINE_TAG_NUMBER(tag_basic_meminfo_t, MULTIBOOT_TAG_TYPE_BASIC_MEMINFO);
DEFINE_TAG_NUMBER(tag_bootdev_t, MULTIBOOT_TAG_TYPE_BOOTDEV);
DEFINE_TAG_NUMBER(tag_mmap_t, MULTIBOOT_TAG_TYPE_MMAP);
DEFINE_TAG_NUMBER(tag_vbe_t, MULTIBOOT_TAG_TYPE_VBE);
DEFINE_TAG_NUMBER(tag_framebuffer_t, MULTIBOOT_TAG_TYPE_FRAMEBUFFER);
DEFINE_TAG_NUMBER(tag_elf_sections_t, MULTIBOOT_TAG_TYPE_ELF_SECTIONS);
DEFINE_TAG_NUMBER(tag_apm_t, MULTIBOOT_TAG_TYPE_APM);
DEFINE_TAG_NUMBER(tag_efi32_t, MULTIBOOT_TAG_TYPE_EFI32);
DEFINE_TAG_NUMBER(tag_efi64_t, MULTIBOOT_TAG_TYPE_EFI64);
DEFINE_TAG_NUMBER(tag_smbios_t, MULTIBOOT_TAG_TYPE_SMBIOS);
DEFINE_TAG_NUMBER(tag_old_acpi_t, MULTIBOOT_TAG_TYPE_ACPI_OLD);
DEFINE_TAG_NUMBER(tag_new_acpi_t, MULTIBOOT_TAG_TYPE_ACPI_NEW);
DEFINE_TAG_NUMBER(tag_network_t, MULTIBOOT_TAG_TYPE_NETWORK);
DEFINE_TAG_NUMBER(tag_efi_mmap_t, MULTIBOOT_TAG_TYPE_EFI_MMAP);
DEFINE_TAG_NUMBER(tag_efi32_ih_t, MULTIBOOT_TAG_TYPE_EFI32_IH);
DEFINE_TAG_NUMBER(tag_efi64_ih_t, MULTIBOOT_TAG_TYPE_EFI64_IH);
DEFINE_TAG_NUMBER(tag_load_base_addr_t, MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR);

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

template <typename Callback>
concept MemoryMapCallback = requires(Callback cb, memory_map_t* entry) {
    { cb(entry) };
};

/**
 * @brief Walk the memory map.
 *
 * @param mmap_tag The memory map tag pointer.
 * @param callback The callback function that accepts a memory map entry.
 */
template <MemoryMapCallback Callback>
void WalkMemoryMap(tag_mmap_t* mmap_tag, Callback callback);

}  // namespace multiboot

#include "extensions.tpp"

#endif  // ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MULTIBOOT2_EXTENSIONS_HPP_
