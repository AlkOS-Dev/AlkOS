#ifndef ALKOS_ALKOS_LIBC_INCLUDE_EXTENSIONS_TEMPLATES_MULTI_ARRAY_HPP_
#define ALKOS_ALKOS_LIBC_INCLUDE_EXTENSIONS_TEMPLATES_MULTI_ARRAY_HPP_

#include "assert.h"
#include "extensions/algorithm.hpp"
#include "extensions/array.hpp"
#include "extensions/concepts.hpp"
#include "extensions/defines.hpp"
#include "extensions/functional.hpp"
#include "extensions/type_traits.hpp"

// TODO: Move out of TemplateLib namespace
// NOTE: Where?

namespace TemplateLib
{

namespace detail::MultiArray
{

constexpr bool MultiplyWillOverflow(size_t a, size_t b)
{
    if (a == 0 || b == 0) {
        return false;
    }
    return a > (static_cast<size_t>(-1) / b);
}

constexpr size_t CalculateProduct(const auto& dims)
{
    if (dims.empty()) {
        return 1;
    }
    size_t product = 1;
    for (size_t dim : dims) {
        R_ASSERT(dim > 0);
        R_ASSERT(!MultiplyWillOverflow(product, dim));

        product *= dim;
    }
    return product;
}

template <size_t N>
constexpr std::array<size_t, N> CalculateStrides(const std::array<size_t, N>& dims)
{
    std::array<size_t, N> strides{};
    if constexpr (N > 0) {
        size_t current_stride = 1;
        for (size_t i = N; i > 0; --i) {
            R_ASSERT(dims[i - 1] > 0);
            R_ASSERT(!MultiplyWillOverflow(current_stride, dims[i - 1]));
            strides[i - 1] = current_stride;
            current_stride *= dims[i - 1];
        }
    }
    return strides;
}

}  // namespace detail::MultiArray

template <typename T, size_t... Dims>
    requires(std::is_default_constructible_v<T>)
class MultiArray
{
    private:
    static_assert(sizeof...(Dims) > 0, "MultiArray must have at least one dimension");
    static_assert(((Dims > 0) && ...), "MultiArray dimensions must be positive");

    //------------------------------------------------------------------------------//
    // Internal Types
    //------------------------------------------------------------------------------//
    using value_t     = T;
    using ref_t       = T&;
    using const_ref_t = const T&;

    static constexpr size_t kRank                    = sizeof...(Dims);
    static constexpr std::array<size_t, kRank> kDims = {Dims...};
    static constexpr size_t kTotalSize               = detail::MultiArray::CalculateProduct(kDims);
    static constexpr std::array<size_t, kRank> kStrides =
        detail::MultiArray::CalculateStrides(kDims);

    using storage_t = std::array<T, kTotalSize>;

    //------------------------------------------------------------------------------//
    // Nested MultiArraySlice class
    //------------------------------------------------------------------------------//

    private:
    template <bool IsConst, size_t kFixedRank>
    class MultiArraySlice
    {
        using ParentPtrType  = std::conditional_t<IsConst, const MultiArray*, MultiArray*>;
        using ElementRefType = std::conditional_t<IsConst, const_ref_t, ref_t>;

        ParentPtrType parent_ptr_;
        size_t base_offset_;

        constexpr MultiArraySlice(ParentPtrType parent_ptr, size_t base_offset) noexcept
            : parent_ptr_(parent_ptr), base_offset_(base_offset)
        {
        }

        friend class MultiArray;

        public:
        MultiArraySlice()                                  = delete;
        MultiArraySlice(const MultiArraySlice&)            = default;
        MultiArraySlice(MultiArraySlice&&)                 = default;
        MultiArraySlice& operator=(const MultiArraySlice&) = delete;
        MultiArraySlice& operator=(MultiArraySlice&&)      = delete;

        constexpr decltype(auto) operator[](size_t next_index) const noexcept
            requires(kFixedRank < kRank - 1)
        {
            R_ASSERT(next_index < parent_ptr_->kDims[kFixedRank]);
            size_t new_offset = base_offset_ + next_index * parent_ptr_->kStrides[kFixedRank];

            return MultiArraySlice<IsConst, kFixedRank + 1>(parent_ptr_, new_offset);
        }

        constexpr decltype(auto) operator[](size_t next_index) const noexcept
            requires(kFixedRank == kRank - 1)
        {
            R_ASSERT(next_index < parent_ptr_->kDims[kFixedRank]);
            size_t flat_index = base_offset_ + next_index;
            return static_cast<ElementRefType>(parent_ptr_->data_[flat_index]);
        }
    };

    public:
    //------------------------------------------------------------------------------//
    // Class Creation and Destruction
    //------------------------------------------------------------------------------//

    /**
     * Creates MultiArray with uninitialized elements.
     */
    constexpr MultiArray() = default;

    /**
     * Creates MultiArray with all elements value-initialized using an initializer list.
     *
     * @param init_list Initializer list with elements to initialize the MultiArray.
     */
    explicit constexpr MultiArray(std::initializer_list<T> init_list)
    {
        R_ASSERT(init_list.size() == kTotalSize);
        std::copy(init_list.begin(), init_list.end(), data_.begin());
    }

    /**
     * Creates Multiarray with all elements value-initialized using a fill value.
     *
     * @param fill_value Value to initialize all elements of the MultiArray.
     */
    explicit constexpr MultiArray(
        const T& fill_value
    ) noexcept(std::is_nothrow_copy_constructible_v<T>)
        : data_{}
    {
        data_.fill(std::forward<const T&>(fill_value));
    }

    /**
     * @brief Creates MultiArray by calling a generator function for each element.
     *
     * The generator function receives the multi-dimensional index of the element
     * as a const reference to a std::array<size_t, kRank>.
     *
     * @tparam Generator The type of the generator function or functor.
     * @param generator A callable entity (function, lambda, functor) that takes
     * `const std::array<size_t, kRank>&` and returns a value
     * convertible to T. It will be called exactly kTotalSize times.
     */
    template <typename Generator>

        requires(
            std::is_invocable_v<Generator, const std::array<size_t, kRank>&> &&
            std::convertible_to<
                std::invoke_result_t<Generator, const std::array<size_t, kRank>&>, T>
        )

    explicit constexpr MultiArray(Generator&& generator) noexcept(

        std::is_nothrow_invocable_v<Generator, const std::array<size_t, kRank>&> &&
        std::is_nothrow_assignable_v<
            T&, std::invoke_result_t<Generator, const std::array<size_t, kRank>&>>

    )
    {
        if constexpr (kTotalSize > 0) {
            std::array<size_t, kRank> current_indices{};  // Start indices at {0, 0, ...}

            // Iterate kTotalSize times, covering every element position
            for (size_t flat_idx = 0; flat_idx < kTotalSize; ++flat_idx) {
                // Call the generator with the current multidimensional indices.
                data_[flat_idx] = std::invoke(std::forward<Generator>(generator), current_indices);

                // Increment the multidimensional indices to handle all permutations
                for (size_t dim_idx = kRank; dim_idx > 0; --dim_idx) {
                    size_t i = dim_idx - 1;
                    current_indices[i]++;
                    if (current_indices[i] < kDims[i]) {
                        break;
                    }
                    current_indices[i] = 0;
                }
            }
        }
    }
    /**
     * Copy constructor.
     *
     * @param other MultiArray to copy from.
     */
    constexpr MultiArray(const MultiArray& other) noexcept(std::is_nothrow_copy_constructible_v<T>)
        requires(std::is_copy_constructible_v<storage_t>)
        : data_(other.data_)
    {
    }

    /**
     * Move constructor.
     *
     * @param other MultiArray to move from.
     */
    constexpr MultiArray(MultiArray&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
        requires(std::is_move_constructible_v<storage_t>)
        : data_(std::move(other.data_))
    {
    }

    /**
     * Copy assignment operator.
     *
     * @param other MultiArray to copy from.
     */
    constexpr MultiArray& operator=(
        const MultiArray& other
    ) noexcept(std::is_nothrow_copy_assignable_v<T>)
        requires(std::is_copy_assignable_v<storage_t>)
    {
        if (this != &other) {
            data_ = other.data_;
        }
        return *this;
    }

    /**
     * Move assignment operator.
     *
     * @param other MultiArray to move from.
     */
    constexpr MultiArray& operator=(
        MultiArray&& other
    ) noexcept(std::is_nothrow_move_assignable_v<T>)
        requires(std::is_move_assignable_v<storage_t>)
    {
        if (this != &other) {
            data_ = std::move(other.data_);
        }
        return *this;
    }

    //------------------------------------------------------------------------------//
    // Public Methods
    //------------------------------------------------------------------------------//

    //------------------------------------------------------------------------------//
    // Public Methods : Chained operator[] Access
    //------------------------------------------------------------------------------//

    [[nodiscard]] constexpr decltype(auto) operator[](size_t index0) noexcept
    {
        R_ASSERT(index0 < kDims[0]);
        if constexpr (kRank == 1) {
            return static_cast<ref_t>(data_[index0]);
        } else {
            size_t offset0 = index0 * kStrides[0];
            return MultiArraySlice<false, 1>(this, offset0);
        }
    }

    [[nodiscard]] constexpr decltype(auto) operator[](size_t index0) const noexcept
    {
        R_ASSERT(index0 < kDims[0]);
        if constexpr (kRank == 1) {
            return static_cast<const_ref_t>(data_[index0]);
        } else {
            size_t offset0 = index0 * kStrides[0];
            return MultiArraySlice<true, 1>(this, offset0);
        }
    }

    //------------------------------------------------------------------------------//
    // Public Methods : Direct Access using operator()
    //------------------------------------------------------------------------------//

    template <typename... IndexTypes>
        requires(sizeof...(IndexTypes) == kRank && (std::convertible_to<IndexTypes, size_t> && ...))
    constexpr T& operator()(IndexTypes... runtime_indices) noexcept
    {
        std::array<size_t, kRank> indices_arr = {static_cast<size_t>(runtime_indices)...};
        size_t flat_index                     = GetFlatIndexChecked(indices_arr);
        return data_[flat_index];
    }

    template <typename... IndexTypes>
        requires(sizeof...(IndexTypes) == kRank && (std::convertible_to<IndexTypes, size_t> && ...))
    constexpr const T& operator()(IndexTypes... runtime_indices) const noexcept
    {
        std::array<size_t, kRank> indices_arr = {static_cast<size_t>(runtime_indices)...};
        size_t flat_index                     = GetFlatIndexChecked(indices_arr);
        return data_[flat_index];
    }

    //------------------------------------------------------------------------------//
    // Public Fields
    //------------------------------------------------------------------------------//
    static constexpr size_t GetRank() noexcept { return kRank; }
    static constexpr size_t GetTotalSize() noexcept { return kTotalSize; }
    static constexpr auto GetDims() noexcept { return kDims; }
    static constexpr auto GetStrides() noexcept { return kStrides; }
    constexpr auto& GetData() noexcept { return data_; }
    constexpr const auto& GetData() const noexcept { return data_; }

    private:
    //------------------------------------------------------------------------------//
    // Private Methods
    //------------------------------------------------------------------------------//

    //------------------------------------------------------------------------------//
    // Private Fields
    //------------------------------------------------------------------------------//
    storage_t data_;

    //------------------------------------------------------------------------------//
    // Helpers
    //------------------------------------------------------------------------------//

    static constexpr size_t GetFlatIndexUnchecked(const std::array<size_t, kRank>& valid_indices)
    {
        size_t flat_index = 0;
        for (size_t i = 0; i < kRank; ++i) {
            flat_index += valid_indices[i] * kStrides[i];
        }
        return flat_index;
    }

    static constexpr size_t GetFlatIndexChecked(const std::array<size_t, kRank>& indices)
    {
        for (size_t i = 0; i < kRank; ++i) {
            R_ASSERT(indices[i] < kDims[i]);
        }
        return GetFlatIndexUnchecked(indices);
    }
};

}  // namespace TemplateLib

#endif  // ALKOS_ALKOS_LIBC_INCLUDE_EXTENSIONS_TEMPLATES_MULTI_ARRAY_HPP_
