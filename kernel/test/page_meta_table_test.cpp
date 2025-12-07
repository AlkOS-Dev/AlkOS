#include <span.hpp>
#include <test_module/test.hpp>

#include <hal/constants.hpp>
#include "mem/page_meta_table.hpp"
#include "mem/types.hpp"

namespace
{
using namespace Mem;
using namespace hal;

constexpr size_t kTotalPages = 100;

class PageMetaTableTest : public TestGroupBase
{
    public:
    PageMetaTableTest() { pmt_.Init({page_meta_buffer_, kTotalPages}); }

    protected:
    PageMetaTable pmt_{};
    PageMeta page_meta_buffer_[kTotalPages]{};
};

TEST_F(PageMetaTableTest, GetPageMeta_WithValidPointer_ReturnsCorrectMetadata)
{
    const auto ptr = PPtr<int>(reinterpret_cast<int *>(kPageSizeBytes * 10));
    PageMeta &meta = pmt_.GetPageMeta(ptr);

    const size_t pfn = PageFrameNumber(ptr);
    R_ASSERT_EQ(&meta, &page_meta_buffer_[pfn]);
}

TEST_F(PageMetaTableTest, GetPageMeta_WithPointerInDifferentPages_ReturnsDifferentMetadata)
{
    const auto ptr1 = PPtr<int>(reinterpret_cast<int *>(kPageSizeBytes * 5));
    const auto ptr2 = PPtr<int>(reinterpret_cast<int *>(kPageSizeBytes * 15));

    PageMeta &meta1 = pmt_.GetPageMeta(ptr1);
    PageMeta &meta2 = pmt_.GetPageMeta(ptr2);

    R_ASSERT_NEQ(&meta1, &meta2);
}

FAIL_TEST_F(PageMetaTableTest, GetPageMeta_WithPointerOutOfBounds_TriggersAssertion)
{
    const auto ptr = PPtr<int>(reinterpret_cast<int *>(kPageSizeBytes * (kTotalPages + 5)));
    (void)pmt_.GetPageMeta(ptr);
}

}  // namespace
