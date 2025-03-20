#include "memory_management/physical_memory_manager.hpp"
#include <extensions/template_lib.hpp>
#include <test_module/test.hpp>
class PhysicalMemoryManagerTest : public TestGroupBase
{
    public:
    static TemplateLib::SingletonInstanceCreator<memory::PhysicalMemoryManager>
        memory_manager_singleton;

    protected:
    void Setup_() override { memory_manager_singleton.Get().SetPageBuffer(buffer_info_); }

    private:
    static unsigned char buffer_[sizeof(u64) * 20];
    static memory::PhysicalMemoryManager::PageBufferInfo_t buffer_info_;
};

unsigned char PhysicalMemoryManagerTest::buffer_[sizeof(u64) * 20];
memory::PhysicalMemoryManager::PageBufferInfo_t PhysicalMemoryManagerTest::buffer_info_{
    .start_addr = reinterpret_cast<u64>(buffer_),
    .size_bytes = sizeof(buffer_),
};
TemplateLib::SingletonInstanceCreator<memory::PhysicalMemoryManager>
    PhysicalMemoryManagerTest::memory_manager_singleton =
        TemplateLib::SingletonInstanceCreator<memory::PhysicalMemoryManager>();
