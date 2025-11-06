#ifndef LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_SPECIAL_MEMBERS_HPP_
#define LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_SPECIAL_MEMBERS_HPP_

namespace template_lib
{
// ------------------------------
// NoCopy
// ------------------------------

struct NoCopy {
    NoCopy()  = default;
    ~NoCopy() = default;

    NoCopy(const NoCopy &) = delete;
    NoCopy(NoCopy &&)      = delete;

    NoCopy &operator=(const NoCopy &) = delete;
    NoCopy &operator=(NoCopy &&)      = delete;
};

// ------------------------------
// MoveOnly
// ------------------------------

struct MoveOnly {
    MoveOnly()  = default;
    ~MoveOnly() = default;

    /* remove copying */
    MoveOnly(const MoveOnly &)            = delete;
    MoveOnly &operator=(const MoveOnly &) = delete;

    /* allow moving */
    MoveOnly(MoveOnly &&)            = default;
    MoveOnly &operator=(MoveOnly &&) = default;
};
}  // namespace template_lib
#endif  // LIBS_LIBTEMPLATE_INCLUDE_TEMPLATE_SPECIAL_MEMBERS_HPP_
