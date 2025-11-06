#ifndef ALKOS_KERNEL_INCLUDE_MODULES_HELPERS_HPP_
#define ALKOS_KERNEL_INCLUDE_MODULES_HELPERS_HPP_

#define DEFINE_MODULE_FIELD(name_space, type) \
                                              \
    private:                                  \
    name_space::type type##_;                 \
                                              \
    public:                                   \
    NODISCARD FORCE_INLINE_F auto &Get##type() noexcept { return type##_; }

#endif  // ALKOS_KERNEL_INCLUDE_MODULES_HELPERS_HPP_
