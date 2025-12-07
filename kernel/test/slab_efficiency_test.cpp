#include <test_module/test.hpp>

#include <template/type_traits.hpp>
#include "mem/phys/mngr/slab_efficiency.hpp"

namespace
{
using namespace Mem::SlabEfficiency;

class SlabEfficiencyTest : public TestGroupBase
{
};

TEST_F(SlabEfficiencyTest, SlabEfficiencyInfo_FreeListItemType_IsMinimalUnsignedType)
{
    using Info1 = SlabEfficiencyInfo<8, 0>;  // 270 capacity -> u16
    EXPECT_TRUE((
        std::is_same_v<
            template_lib::minimal_unsigned_type_t<Info1::kCapacity>, Info1::FreeListItemType>
    ));

    using Info2 = SlabEfficiencyInfo<2000, 0>;  // 2 capacity -> u8
    EXPECT_TRUE((
        std::is_same_v<
            template_lib::minimal_unsigned_type_t<Info2::kCapacity>, Info2::FreeListItemType>
    ));
}

TEST_F(SlabEfficiencyTest, OperatorLess_WhenEfficiencyDiffersSignificantly_SortsByEfficiency)
{
    EfficiencyData d1{.efficiency = 0.8, .block_order = 1};
    EfficiencyData d2{.efficiency = 0.9, .block_order = 0};
    EXPECT_TRUE(d1 < d2);
    EXPECT_FALSE(d2 < d1);
}

TEST_F(SlabEfficiencyTest, OperatorLess_WhenEfficiencyIsSimilar_SortsByBlockOrderDescending)
{
    EfficiencyData d1{.efficiency = 0.850, .block_order = 1};
    EfficiencyData d2{.efficiency = 0.855, .block_order = 2};
    EXPECT_TRUE(d2 < d1);
    EXPECT_FALSE(d1 < d2);
}

}  // namespace
