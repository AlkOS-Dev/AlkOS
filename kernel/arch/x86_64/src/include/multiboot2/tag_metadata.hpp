#ifndef KERNEL_ARCH_X86_64_SRC_INCLUDE_MULTIBOOT2_TAG_METADATA_HPP_
#define KERNEL_ARCH_X86_64_SRC_INCLUDE_MULTIBOOT2_TAG_METADATA_HPP_

#include <bit.hpp>
#include <defines.hpp>

#include "include/multiboot2/multiboot2.h"

namespace Multiboot
{

static constexpr u32 kInvalidTagNumber = kFullMask<u32>;

// Primary template defaults to 0. This can't be enforced with a concept because
// tags do not have a common base class. (Framebuffer)
template <class TagType>
struct TagMetadata {
    static constexpr u32 kValue           = kInvalidTagNumber;
    static constexpr const char *kTagName = "INVALID_TAG";
};

// Macro to conveniently specialize TagNumber for a given tag structure and its constant.
#define DEFINE_TAG_METADATA(TagStruct, TagValue)           \
    template <>                                            \
    struct TagMetadata<TagStruct> {                        \
        static constexpr u32 kValue           = TagValue;  \
        static constexpr const char *kTagName = #TagValue; \
    }

// Specializations for each tag type based on the kMultibootTagType constants.
// NOTE: This is not ideal - the defines are not 1:1 (Eg. TagString is not the same as
// kMultibootTagTypeCmdline) TagString can have kMultibootTagTypeCmdline or
// kMultibootTagTypeBootLoaderName, But this is unimportant for the current use case.
DEFINE_TAG_METADATA(TagString, kMultibootTagTypeCmdline);
DEFINE_TAG_METADATA(TagModule, kMultibootTagTypeModule);
DEFINE_TAG_METADATA(TagBasicMeminfo, kMultibootTagTypeBasicMeminfo);
DEFINE_TAG_METADATA(TagBootdev, kMultibootTagTypeBootdev);
DEFINE_TAG_METADATA(TagMmap, kMultibootTagTypeMmap);
DEFINE_TAG_METADATA(TagVbe, kMultibootTagTypeVbe);
DEFINE_TAG_METADATA(TagFramebuffer, kMultibootTagTypeFramebuffer);
DEFINE_TAG_METADATA(TagElfSections, kMultibootTagTypeElfSections);
DEFINE_TAG_METADATA(TagApm, kMultibootTagTypeApm);
DEFINE_TAG_METADATA(TagEfi32, kMultibootTagTypeEfi32);
DEFINE_TAG_METADATA(TagEfi64, kMultibootTagTypeEfi64);
DEFINE_TAG_METADATA(TagSmbios, kMultibootTagTypeSmbios);
DEFINE_TAG_METADATA(TagOldAcpi, kMultibootTagTypeAcpiOld);
DEFINE_TAG_METADATA(TagNewAcpi, kMultibootTagTypeAcpiNew);
DEFINE_TAG_METADATA(TagNetwork, kMultibootTagTypeNetwork);
DEFINE_TAG_METADATA(TagEfiMmap, kMultibootTagTypeEfiMmap);
DEFINE_TAG_METADATA(TagEfi32Ih, kMultibootTagTypeEfi32Ih);
DEFINE_TAG_METADATA(TagEfi64Ih, kMultibootTagTypeEfi64Ih);
DEFINE_TAG_METADATA(TagLoadBaseAddr, kMultibootTagTypeLoadBaseAddr);

}  // namespace Multiboot

#endif  // KERNEL_ARCH_X86_64_SRC_INCLUDE_MULTIBOOT2_TAG_METADATA_HPP_
