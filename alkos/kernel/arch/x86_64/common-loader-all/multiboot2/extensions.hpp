#ifndef ALK_OS_KERNEL_ARCH_X86_64_BOOT32_INCLUDE_MULTIBOOT2_EXTENSIONS_HPP_
#define ALK_OS_KERNEL_ARCH_X86_64_BOOT32_INCLUDE_MULTIBOOT2_EXTENSIONS_HPP_

#include <multiboot2/multiboot2.h>
#include <todo.hpp>
#include <types.hpp>

namespace multiboot
{

// callback that returns true if the tag is the one we are looking for and false otherwise
TODO_WHEN_TYPETRAITS_MERGED
// Add -> std::convertible_to<bool> to the concept
template <class FilterT, class TagT>
concept TagFilter = requires(FilterT filter, TagT* tag) {
    { filter(tag) };
};

// Primary template defaults to 0. This can't be enforced with a concept because
// tags do not have a common base class. (framebuffer_t)
template <class TagType>
struct TagNumber {
    static constexpr u32 kInvalidTagNumber = ~0u;
    static constexpr u32 value             = kInvalidTagNumber;
    static constexpr const char* kTagName  = "INVALID_TAG";
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
 * @brief Get the tag name from the tag type.
 *
 * @param type The tag type.
 * @return const char* The tag name.
 */
const char* GetTagName(u32 type);

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

#endif  // ALK_OS_KERNEL_ARCH_X86_64_BOOT32_INCLUDE_MULTIBOOT2_EXTENSIONS_HPP_
