// TODO
// #include <extensions/template_lib.hpp>
// #include <test_module/test.hpp>
// #include "memory_management/physical_memory_manager.hpp"
// class PhysicalMemoryManagerTest : public TestGroupBase
// {
//     protected:
//     static template_lib::SingletonInstanceCreator<memory::PhysicalMemoryManager>
//         memory_manager_singleton;
//
//     void Setup_() override
//     {
//         for (size_t i = 0; i < sizeof(buffer_); ++i) {
//             buffer_[i] = i;
//         }
//         memory_manager_singleton.Init(buffer_info_);
//     }
//     static unsigned char buffer_[sizeof(u64) * 20];
//
//     private:
//     static memory::PhysicalMemoryManager::PageBufferInfo_t buffer_info_;
// };
//
// unsigned char PhysicalMemoryManagerTest::buffer_[sizeof(u64) * 20];
// memory::PhysicalMemoryManager::PageBufferInfo_t PhysicalMemoryManagerTest::buffer_info_{
//     .start_addr = reinterpret_cast<u64>(buffer_),
//     .size_bytes = sizeof(buffer_),
// };
// template_lib::SingletonInstanceCreator<memory::PhysicalMemoryManager>
//     PhysicalMemoryManagerTest::memory_manager_singleton =
//         template_lib::SingletonInstanceCreator<memory::PhysicalMemoryManager>();
//
// TEST_F(PhysicalMemoryManagerTest, Free_WhenCalled_ShouldIncreaseNumFreePages)
// {
//     // Given
//     ASSERT_EQ(memory_manager_singleton.Get().GetNumFreePages(), 0_u64);
//     const auto page_address = buffer_[0];
//
//     // When
//     memory_manager_singleton.Get().Free(page_address);
//
//     // Then
//     ASSERT_EQ(memory_manager_singleton.Get().GetNumFreePages(), 1);
// }
//
// TEST_F(PhysicalMemoryManagerTest, Allocate_WhenCalled_ShouldDecreaseNumFreePages)
// {
//     // Given
//     auto page_address = buffer_[0];
//     memory_manager_singleton.Get().Free(page_address);
//     ASSERT_EQ(memory_manager_singleton.Get().GetNumFreePages(), 1);
//
//     // When
//     page_address = memory_manager_singleton.Get().Allocate();
//
//     // Then
//     ASSERT_EQ(memory_manager_singleton.Get().GetNumFreePages(), 0);
// }
//
// TEST_F(PhysicalMemoryManagerTest, Allocate_WhenCalled_ShouldReturnCorrectPageAddress)
// {
//     // Given
//     const uintptr_t test_page_address = buffer_[0];
//     memory_manager_singleton.Get().Free(test_page_address);
//
//     // When
//     const auto page_address = memory_manager_singleton.Get().Allocate();
//
//     // Then
//     ASSERT_EQ(page_address, test_page_address);
// }
