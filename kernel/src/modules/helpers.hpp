#ifndef KERNEL_SRC_MODULES_HELPERS_HPP_
#define KERNEL_SRC_MODULES_HELPERS_HPP_

#define DEFINE_MODULE_FIELD(name_space, type) \
                                              \
    private:                                  \
    name_space::type type##_;                 \
                                              \
    public:                                   \
    NODISCARD FORCE_INLINE_F auto &Get##type() noexcept { return type##_; }

#endif  // KERNEL_SRC_MODULES_HELPERS_HPP_
