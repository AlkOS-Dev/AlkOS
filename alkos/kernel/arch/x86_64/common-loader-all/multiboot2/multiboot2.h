#ifndef ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MULTIBOOT2_MULTIBOOT2_H_
#define ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MULTIBOOT2_MULTIBOOT2_H_

#include "extensions/types.hpp"

namespace multiboot
{

/*   multiboot2.h - Multiboot 2 header file. */
/*   Copyright (C) 1999,2003,2007,2008,2009,2010  Free Software Foundation, Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ANY
 *  DEVELOPER OR DISTRIBUTOR BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef MULTIBOOT_HEADER
#define MULTIBOOT_HEADER 1

// How many bytes from the start of the file we search for the header.
static constexpr u32 kMultibootSearch      = 32768;
static constexpr u32 kMultibootHeaderAlign = 8;

// The magic field should contain this.
static constexpr u32 kMultiboot2HeaderMagic = 0xe85250d6;

// This should be in %eax.
static constexpr u32 kMultiboot2BootloaderMagic = 0x36d76289;

// Alignment of multiboot modules.
static constexpr u32 kMultibootModAlign = 0x00001000;

// Alignment of the multiboot info structure.
static constexpr u32 kMultibootInfoAlign = 0x00000008;

// Flags set in the \'flags\' member of the multiboot header.
static constexpr u32 kMultibootTagAlign              = 8;
static constexpr u32 kMultibootTagTypeEnd            = 0;
static constexpr u32 kMultibootTagTypeCmdline        = 1;
static constexpr u32 kMultibootTagTypeBootLoaderName = 2;
static constexpr u32 kMultibootTagTypeModule         = 3;
static constexpr u32 kMultibootTagTypeBasicMeminfo   = 4;
static constexpr u32 kMultibootTagTypeBootdev        = 5;
static constexpr u32 kMultibootTagTypeMmap           = 6;
static constexpr u32 kMultibootTagTypeVbe            = 7;
static constexpr u32 kMultibootTagTypeFramebuffer    = 8;
static constexpr u32 kMultibootTagTypeElfSections    = 9;
static constexpr u32 kMultibootTagTypeApm            = 10;
static constexpr u32 kMultibootTagTypeEfi32          = 11;
static constexpr u32 kMultibootTagTypeEfi64          = 12;
static constexpr u32 kMultibootTagTypeSmbios         = 13;
static constexpr u32 kMultibootTagTypeAcpiOld        = 14;
static constexpr u32 kMultibootTagTypeAcpiNew        = 15;
static constexpr u32 kMultibootTagTypeNetwork        = 16;
static constexpr u32 kMultibootTagTypeEfiMmap        = 17;
static constexpr u32 kMultibootTagTypeEfiBs          = 18;
static constexpr u32 kMultibootTagTypeEfi32Ih        = 19;
static constexpr u32 kMultibootTagTypeEfi64Ih        = 20;
static constexpr u32 kMultibootTagTypeLoadBaseAddr   = 21;

static constexpr u32 kMultibootHeaderTagEnd                = 0;
static constexpr u32 kMultibootHeaderTagInformationRequest = 1;
static constexpr u32 kMultibootHeaderTagAddress            = 2;
static constexpr u32 kMultibootHeaderTagEntryAddress       = 3;
static constexpr u32 kMultibootHeaderTagConsoleFlags       = 4;
static constexpr u32 kMultibootHeaderTagFramebuffer        = 5;
static constexpr u32 kMultibootHeaderTagModuleAlign        = 6;
static constexpr u32 kMultibootHeaderTagEfiBs              = 7;
static constexpr u32 kMultibootHeaderTagEntryAddressEfi32  = 8;
static constexpr u32 kMultibootHeaderTagEntryAddressEfi64  = 9;
static constexpr u32 kMultibootHeaderTagRelocatable        = 10;

static constexpr u32 kMultibootArchitectureI386   = 0;
static constexpr u32 kMultibootArchitectureMips32 = 4;
static constexpr u32 kMultibootHeaderTagOptional  = 1;

static constexpr u32 kMultibootLoadPreferenceNone = 0;
static constexpr u32 kMultibootLoadPreferenceLow  = 1;
static constexpr u32 kMultibootLoadPreferenceHigh = 2;

static constexpr u32 kMultibootConsoleFlagsConsoleRequired  = 1;
static constexpr u32 kMultibootConsoleFlagsEgaTextSupported = 2;

#ifndef ASM_FILE

struct header_t {
    /*  Must be MULTIBOOT_MAGIC - see above. */
    u32 magic;

    /*  ISA */
    u32 architecture;

    /*  Total header length. */
    u32 header_length;

    /*  The above fields plus this one must equal 0 mod 2^32. */
    u32 checksum;
};

struct header_tag_t {
    u16 type;
    u16 flags;
    u32 size;
};

struct header_tag_information_request_t {
    u16 type;
    u16 flags;
    u32 size;
    u32 requests[0];
};

struct header_tag_address_t {
    u16 type;
    u16 flags;
    u32 size;
    u32 header_addr;
    u32 load_addr;
    u32 load_end_addr;
    u32 bss_end_addr;
};

struct header_tag_entry_address_t {
    u16 type;
    u16 flags;
    u32 size;
    u32 entry_addr;
};

struct header_tag_console_flags_t {
    u16 type;
    u16 flags;
    u32 size;
    u32 console_flags;
};

struct header_tag_framebuffer_t {
    u16 type;
    u16 flags;
    u32 size;
    u32 width;
    u32 height;
    u32 depth;
};

struct header_tag_module_align_t {
    u16 type;
    u16 flags;
    u32 size;
};

struct header_tag_relocatable_t {
    u16 type;
    u16 flags;
    u32 size;
    u32 min_addr;
    u32 max_addr;
    u32 align;
    u32 preference;
};

struct color_t {
    u8 red;
    u8 green;
    u8 blue;
};

struct mmap_entry_t {
    u64 addr;
    u64 len;
    u32 type;
    u32 zero;
    static constexpr u32 kMemoryAvailable       = 1;
    static constexpr u32 kMemoryReserved        = 2;
    static constexpr u32 kMemoryAcpiReclaimable = 3;
    static constexpr u32 kMemoryNvs             = 4;
    static constexpr u32 kMemoryBadram          = 5;
};
typedef struct mmap_entry_t memory_map_t;

struct tag_t {
    u32 type;
    u32 size;
};

struct tag_string_t {
    u32 type;
    u32 size;
    char string[0];
};

struct tag_module_t {
    u32 type;
    u32 size;
    u32 mod_start;
    u32 mod_end;
    char cmdline[0];
};

struct tag_basic_meminfo_t {
    u32 type;
    u32 size;
    u32 mem_lower;
    u32 mem_upper;
};

struct tag_bootdev_t {
    u32 type;
    u32 size;
    u32 biosdev;
    u32 slice;
    u32 part;
};

struct tag_mmap_t {
    u32 type;
    u32 size;
    u32 entry_size;
    u32 entry_version;
    struct mmap_entry_t entries[0];
};

struct vbe_info_block_t {
    u8 external_specification[512];
};

struct vbe_mode_info_block_t {
    u8 external_specification[256];
};

struct tag_vbe_t {
    u32 type;
    u32 size;

    u16 vbe_mode;
    u16 vbe_interface_seg;
    u16 vbe_interface_off;
    u16 vbe_interface_len;

    struct vbe_info_block_t vbe_control_info;
    struct vbe_mode_info_block_t vbe_mode_info;
};

struct tag_framebuffer_common_t {
    u32 type;
    u32 size;

    u64 framebuffer_addr;
    u32 framebuffer_pitch;
    u32 framebuffer_width;
    u32 framebuffer_height;
    u8 framebuffer_bpp;
#define MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED  0
#define MULTIBOOT_FRAMEBUFFER_TYPE_RGB      1
#define MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT 2
    u8 framebuffer_type;
    u16 reserved;
};

struct tag_framebuffer_t {
    struct tag_framebuffer_common_t common;

    union {
        struct {
            u16 framebuffer_palette_num_colors;
            struct color_t framebuffer_palette[0];
        };
        struct {
            u8 framebuffer_red_field_position;
            u8 framebuffer_red_mask_size;
            u8 framebuffer_green_field_position;
            u8 framebuffer_green_mask_size;
            u8 framebuffer_blue_field_position;
            u8 framebuffer_blue_mask_size;
        };
    };
};

struct tag_elf_sections_t {
    u32 type;
    u32 size;
    u32 num;
    u32 entsize;
    u32 shndx;
    char sections[0];
};

struct tag_apm_t {
    u32 type;
    u32 size;
    u16 version;
    u16 cseg;
    u32 offset;
    u16 cseg_16;
    u16 dseg;
    u16 flags;
    u16 cseg_len;
    u16 cseg_16_len;
    u16 dseg_len;
};

struct tag_efi32_t {
    u32 type;
    u32 size;
    u32 pointer;
};

struct tag_efi64_t {
    u32 type;
    u32 size;
    u64 pointer;
};

struct tag_smbios_t {
    u32 type;
    u32 size;
    u8 major;
    u8 minor;
    u8 reserved[6];
    u8 tables[0];
};

struct tag_old_acpi_t {
    u32 type;
    u32 size;
    u8 rsdp[0];
};

struct tag_new_acpi_t {
    u32 type;
    u32 size;
    u8 rsdp[0];
};

struct tag_network_t {
    u32 type;
    u32 size;
    u8 dhcpack[0];
};

struct tag_efi_mmap_t {
    u32 type;
    u32 size;
    u32 descr_size;
    u32 descr_vers;
    u8 efi_mmap[0];
};

struct tag_efi32_ih_t {
    u32 type;
    u32 size;
    u32 pointer;
};

struct tag_efi64_ih_t {
    u32 type;
    u32 size;
    u64 pointer;
};

struct tag_load_base_addr_t {
    u32 type;
    u32 size;
    u32 load_base_addr;
};

#endif /*  ! ASM_FILE */

#endif /*  ! MULTIBOOT_HEADER */

}  // namespace multiboot

#endif  // ALKOS_ALKOS_KERNEL_ARCH_X86_64_COMMON_LOADER_ALL_MULTIBOOT2_MULTIBOOT2_H_
