#include <multiboot2/multiboot2.h>
#include <multiboot2/extensions.hpp>
#include <terminal.hpp>
#include <types.hpp>
#include "debug.hpp"

namespace multiboot
{

const char *GetTagName(u32 type)
{
    switch (type) {
        case MULTIBOOT_TAG_TYPE_END:
            return "MULTIBOOT_TAG_TYPE_END";
        case MULTIBOOT_TAG_TYPE_CMDLINE:
            return "MULTIBOOT_TAG_TYPE_CMDLINE";
        case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
            return "MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME";
        case MULTIBOOT_TAG_TYPE_MODULE:
            return "MULTIBOOT_TAG_TYPE_MODULE";
        case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
            return "MULTIBOOT_TAG_TYPE_BASIC_MEMINFO";
        case MULTIBOOT_TAG_TYPE_BOOTDEV:
            return "MULTIBOOT_TAG_TYPE_BOOTDEV";
        case MULTIBOOT_TAG_TYPE_MMAP:
            return "MULTIBOOT_TAG_TYPE_MMAP";
        case MULTIBOOT_TAG_TYPE_VBE:
            return "MULTIBOOT_TAG_TYPE_VBE";
        case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
            return "MULTIBOOT_TAG_TYPE_FRAMEBUFFER";
        case MULTIBOOT_TAG_TYPE_ELF_SECTIONS:
            return "MULTIBOOT_TAG_TYPE_ELF_SECTIONS";
        case MULTIBOOT_TAG_TYPE_APM:
            return "MULTIBOOT_TAG_TYPE_APM";
        case MULTIBOOT_TAG_TYPE_EFI32:
            return "MULTIBOOT_TAG_TYPE_EFI32";
        case MULTIBOOT_TAG_TYPE_EFI64:
            return "MULTIBOOT_TAG_TYPE_EFI64";
        case MULTIBOOT_TAG_TYPE_SMBIOS:
            return "MULTIBOOT_TAG_TYPE_SMBIOS";
        case MULTIBOOT_TAG_TYPE_ACPI_OLD:
            return "MULTIBOOT_TAG_TYPE_ACPI_OLD";
        case MULTIBOOT_TAG_TYPE_ACPI_NEW:
            return "MULTIBOOT_TAG_TYPE_ACPI_NEW";
        case MULTIBOOT_TAG_TYPE_NETWORK:
            return "MULTIBOOT_TAG_TYPE_NETWORK";
        case MULTIBOOT_TAG_TYPE_EFI_MMAP:
            return "MULTIBOOT_TAG_TYPE_EFI_MMAP";
        case MULTIBOOT_TAG_TYPE_EFI_BS:
            return "MULTIBOOT_TAG_TYPE_EFI_BS";
        case MULTIBOOT_TAG_TYPE_EFI32_IH:
            return "MULTIBOOT_TAG_TYPE_EFI32_IH";
        case MULTIBOOT_TAG_TYPE_EFI64_IH:
            return "MULTIBOOT_TAG_TYPE_EFI64_IH";
        case MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR:
            return "MULTIBOOT_TAG_TYPE_LOAD_BASE_ADDR";
        default:
            return "UNKNOWN";
    }
}

}  // namespace multiboot
