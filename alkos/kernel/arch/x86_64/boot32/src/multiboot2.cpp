#include <multiboot2.h>
#include <tags.hpp>
#include <terminal.hpp>
#include "debug.hpp"
#include "extensions/types.hpp"

extern char text_buffer[1024];

multiboot_tag_module *FindKernelModule(void *multiboot_info_addr)
{
    multiboot_tag_module *kernel_module = nullptr;
    for (auto *tag =
             reinterpret_cast<multiboot_tag *>(static_cast<char *>(multiboot_info_addr) + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = reinterpret_cast<multiboot_tag *>(
             reinterpret_cast<multiboot_uint8_t *>(tag) + ((tag->size + 7) & ~7)
         )) {
        const char *tag_name = GetTagName(tag->type);
        TRACE_INFO("Found tag: %s", tag_name);

        if (tag->type == MULTIBOOT_TAG_TYPE_MODULE) {
            auto *module  = reinterpret_cast<multiboot_tag_module *>(tag);
            kernel_module = module;

            // TODO: Modules should be loaded with a specific command line
            // so that the loader knows which module is the kernel
            // but for now, we will just take the last module as the kernel

            TRACE_INFO(
                "Module loaded at: 0x%X - 0x%X", static_cast<uint32_t>(module->mod_start),
                static_cast<uint32_t>(module->mod_end)
            );

            TRACE_INFO(
                "With command line: %s",
                (module->cmdline && *static_cast<char *>(module->cmdline) != '\0')
                    ? static_cast<char *>(module->cmdline)
                    : "no command line provided"
            );
        }
    }
    return kernel_module;
}

const char *GetTagName(unsigned int type)
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
