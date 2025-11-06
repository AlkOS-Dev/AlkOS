#ifndef ALKOS_KERNEL_INCLUDE_MEM_VIRT_ADDR_SPACE_ITERATOR_HPP_
#define ALKOS_KERNEL_INCLUDE_MEM_VIRT_ADDR_SPACE_ITERATOR_HPP_

#include <extensions/expected.hpp>
#include <extensions/types.hpp>

#include "mem/error.hpp"
#include "mem/types.hpp"
#include "mem/virt/area.hpp"

namespace Mem
{

//==============================================================================
// Forward Declarations
//==============================================================================

class VirtualMemoryManager;
class AddressSpace;

//==============================================================================
// AddressSpaceConstIterator
//==============================================================================

class AddressSpaceConstIterator
{
    public:
    using value_type      = VMemArea;
    using difference_type = std::ptrdiff_t;
    using pointer         = VPtr<const VMemArea>;
    using reference       = const VMemArea &;

    explicit AddressSpaceConstIterator(pointer ptr) : m_ptr_(ptr) {}

    reference operator*() const { return *m_ptr_; }
    pointer operator->() const { return m_ptr_; }

    AddressSpaceConstIterator &operator++()
    {
        if (m_ptr_) {
            m_ptr_ = m_ptr_->next;
        }
        return *this;
    }

    AddressSpaceConstIterator operator++(int)
    {
        AddressSpaceConstIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    friend bool operator==(const AddressSpaceConstIterator &a, const AddressSpaceConstIterator &b)
    {
        return a.m_ptr_ == b.m_ptr_;
    }
    friend bool operator!=(const AddressSpaceConstIterator &a, const AddressSpaceConstIterator &b)
    {
        return a.m_ptr_ != b.m_ptr_;
    }

    private:
    pointer m_ptr_;
};

using AddrSpIt = AddressSpaceConstIterator;

}  // namespace Mem

#endif  // ALKOS_KERNEL_INCLUDE_MEM_VIRT_ADDR_SPACE_ITERATOR_HPP_
