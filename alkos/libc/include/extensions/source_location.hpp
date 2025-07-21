#ifndef ALKOS_LIBC_INCLUDE_EXTENSIONS_SOURCE_LOCATION_HPP_
#define ALKOS_LIBC_INCLUDE_EXTENSIONS_SOURCE_LOCATION_HPP_

namespace std
{

struct source_location {
    // ------------------------------
    // Member types
    // ------------------------------

    private:
    struct __impl {
        const char *_M_file_name;
        const char *_M_function_name;
        uint_least32_t _M_line;
        uint_least32_t _M_column;
    };
    using builtin_type = decltype(__builtin_source_location());

    public:
    // ------------------------------
    // Class creation
    // ------------------------------
    constexpr source_location() noexcept         = default;
    source_location(const source_location &)     = default;
    source_location(source_location &&) noexcept = default;

    // ------------------------------
    // Operators
    // ------------------------------
    source_location &operator=(const source_location &)     = default;
    source_location &operator=(source_location &&) noexcept = default;

    // ------------------------------
    // Fields
    // ------------------------------
    constexpr const char *file_name() const noexcept
    {
        return location_ ? location_->_M_file_name : "";
    }
    constexpr const char *function_name() const noexcept
    {
        return location_ ? location_->_M_function_name : "";
    }
    constexpr uint_least32_t line() const noexcept { return location_ ? location_->_M_line : 0; }
    constexpr uint_least32_t column() const noexcept
    {
        return location_ ? location_->_M_column : 0;
    }

    // ------------------------------
    // Static methods
    // ------------------------------
    static consteval source_location current(builtin_type _p = __builtin_source_location()) noexcept
    {
        source_location sl;
        sl.location_ = static_cast<const __impl *>(_p);
        return sl;
    }

    // ------------------------------
    // Private members
    // ------------------------------

    private:
    const __impl *location_ = nullptr;
};

}  // namespace std

#endif  // ALKOS_LIBC_INCLUDE_EXTENSIONS_SOURCE_LOCATION_HPP_
