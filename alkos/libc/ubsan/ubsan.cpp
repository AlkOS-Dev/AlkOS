#include "extensions/bit.hpp"
#include "extensions/debug.hpp"
#include "extensions/defines.hpp"
#include "extensions/internal/macro.hpp"
#include "extensions/utility.hpp"

#include <panic.hpp>

/////////////////////////////
//       Constants         //
/////////////////////////////

static constexpr const char* kTypeCheckKinds[] = {
    "Load operation on",     "Store operation to",      "Reference binding to",
    "Member access within",  "Member function call on", "Constructor invocation on",
    "Downcast operation on", "Secondary downcast on",   "Upcast operation on",
    "Virtual base cast on",  "Non-null binding to",     "Dynamic operation on"
};

static constexpr const char* kOutOfBoundsKinds[] = {
    "Array index out of bounds", "Pointer subtraction out of bounds",
    "Pointer addition out of bounds", "Pointer offset out of bounds"
};

static constexpr const char* kBuiltinCheckKinds[] = {
    "Count trailing zeros of zero", "Count leading zeros of zero", "Assume condition false"
};

static constexpr const char* kCFITypeCheckKinds[] = {
    "Virtual call",
    "Non-virtual call",
    "Derived cast",
    "Unrelated cast",
    "Indirect call",
    "Non-virtual member function call",
    "Virtual member function call"
};

////////////////////////////
//        Structs         //
////////////////////////////

struct UbsanSourceLocation {
    const char* file;
    u32 line;
    u32 column;
};

struct UbsanTypeDescriptor {
    u16 kind;
    u16 info;
    char name[];
};

struct UbsanTypeMismatchData {
    UbsanSourceLocation location;
    const UbsanTypeDescriptor* type;
    u8 alignment;
    u8 type_check_kind;
};

struct UbsanAlignmentAssumptionData {
    UbsanSourceLocation location;
    UbsanSourceLocation assumption_location;
    const UbsanTypeDescriptor* type;
};

struct UbsanOverflowData {
    UbsanSourceLocation location;
    const UbsanTypeDescriptor* type;
};

struct UbsanShiftOutOfBoundsData {
    UbsanSourceLocation location;
    const UbsanTypeDescriptor* lhs_type;
    const UbsanTypeDescriptor* rhs_type;
};

struct UbsanOutOfBoundsData {
    UbsanSourceLocation location;
    const UbsanTypeDescriptor* left_type;
    const UbsanTypeDescriptor* right_type;
};

struct UbsanUnreachableData {
    UbsanSourceLocation location;
};

struct UbsanVLABoundData {
    UbsanSourceLocation location;
    const UbsanTypeDescriptor* type;
};

struct UbsanFloatCastOverflowData {
    UbsanSourceLocation location;
    const UbsanTypeDescriptor* from_type;
    const UbsanTypeDescriptor* to_type;
};

struct UbsanInvalidValueData {
    UbsanSourceLocation location;
    const UbsanTypeDescriptor* type;
};

struct UbsanImplicitConversionData {
    UbsanSourceLocation location;
    const UbsanTypeDescriptor* from_type;
    const UbsanTypeDescriptor* to_type;
    u8 kind;
    u32 bitfield_bits;
};

struct UbsanInvalidBuiltinData {
    UbsanSourceLocation location;
    u8 kind;
};

struct UbsanInvalidObjCCastData {
    UbsanSourceLocation location;
    const UbsanTypeDescriptor* expected_type;
};

struct UbsanNonNullReturnData {
    UbsanSourceLocation attr_location;
};

struct UbsanNonNullArgData {
    UbsanSourceLocation location;
    UbsanSourceLocation attr_location;
    i32 arg_index;
};

struct UbsanPointerOverflowData {
    UbsanSourceLocation location;
};

struct UbsanCFIBadIcallData {
    UbsanSourceLocation location;
    const UbsanTypeDescriptor* type;
};

struct UbsanCFICheckFailData {
    u8 check_kind;
    UbsanSourceLocation location;
    const UbsanTypeDescriptor* type;
};

struct UbsanReportOptions {
    bool from_unrecoverable_handler;
    uintptr_t pc, bp;
};

struct UbsanFunctionTypeMismatchData {
    UbsanSourceLocation location;
    const UbsanTypeDescriptor* type;
};

////////////////////////////
//   Reporting Helpers    //
////////////////////////////

constexpr UbsanSourceLocation kUnknownLocation = {"<Unknown File>", 0, 0};

template <typename... Args>
static int FormatUbsanMessage(
    char* buffer, size_t buffer_size, const char* violation_type,
    const UbsanSourceLocation* location, const char* format = nullptr, Args... args
)
{
    if (buffer == nullptr || buffer_size == 0) {
        return 0;
    }

    if (location == nullptr) {
        location = &kUnknownLocation;
    }

    int written = snprintf(
        buffer, buffer_size,
        "UBSAN: %s\n"
        "File: %s\n"
        "Line: %u\n"
        "Column: %u\n",
        violation_type, location->file, location->line, location->column
    );

    if (format != nullptr && written > 0 && static_cast<size_t>(written) < buffer_size - 1) {
        written += snprintf(buffer + written, buffer_size - written, "Details:\n");
        written += snprintf(buffer + written, buffer_size - written, format, args...);
    }

    // Add final newline if space permits
    if (static_cast<size_t>(written) < buffer_size - 1) {
        buffer[written++] = '\n';
        buffer[written]   = '\0';
    } else {
        buffer[buffer_size - 1] = '\0';
        written                 = buffer_size - 1;
    }

    return written;
}

template <bool fatal, typename... Args>
static void UbsanReport(
    const char* violation_type, const UbsanSourceLocation* location, const char* format = nullptr,
    Args... args
)
{
    char message_buffer[1024];
    FormatUbsanMessage(
        message_buffer, sizeof(message_buffer), violation_type, location, format, args...
    );

    if constexpr (fatal) {
        if constexpr (kIsKernel) {
            KernelPanicFormat(message_buffer);
        } else {
            TODO_USERSPACE
            // TODO(F1r3d3v): Implement user space panic/process termination
        }
    }

    TODO_LIBK_TRACE
    // TODO(F1r3d3v): Enable tracing when using libk
    TRACE_WARNING(message_buffer);
}

////////////////////////////
//     Implementation     //
////////////////////////////

#define MERGE(x, y)              x y
#define GET_SECOND(x, y)         y
#define FOR_EACH_MERGE(...)      FOR_EACH_PAIR(MERGE, __VA_ARGS__)
#define FOR_EACH_GET_SECOND(...) FOR_EACH_PAIR(GET_SECOND, __VA_ARGS__)
#define GET_FIRST(x, ...)        x

#define UBSAN_ABORT_VARIANT(checkname, ...)                                            \
    DECL_C NO_RET void __ubsan_handle_##checkname##_abort(FOR_EACH_MERGE(__VA_ARGS__)) \
    {                                                                                  \
        /* Call the base handler to report the violation */                            \
        __ubsan_handle_##checkname(FOR_EACH_GET_SECOND(__VA_ARGS__));                  \
                                                                                       \
        /* Now trigger the panic/abort sequence */                                     \
        if constexpr (kIsKernel) {                                                     \
            KernelPanic("UBSAN: Unrecoverable error");                                 \
        } else {                                                                       \
            TODO_USERSPACE                                                             \
            /* TODO(F1r3d3v): Implement user space panic/process termination */        \
        }                                                                              \
                                                                                       \
        /* Should not be reached */                                                    \
        std::unreachable();                                                            \
    }

// --- Type Mismatch ---
DECL_C void __ubsan_handle_type_mismatch_v1(UbsanTypeMismatchData* data, uintptr_t ptr)
{
    const char* violation_base;
    const char* type_check_kind_str = data->type_check_kind < ARRAY_SIZE(kTypeCheckKinds)
                                          ? kTypeCheckKinds[data->type_check_kind]
                                          : "Unknown";

    if (ptr == 0) {
        violation_base = "Null pointer access";
        UbsanReport<false>(violation_base, &data->location);
    } else if (data->alignment != 0 && !IsAligned(ptr, data->alignment)) {
        violation_base = "Unaligned memory access";
        UbsanReport<false>(
            violation_base, &data->location, "%s address %p, Required Alignment: %lu",
            type_check_kind_str, reinterpret_cast<void*>(ptr), data->alignment
        );
    } else {
        violation_base = "Insufficient size";
        UbsanReport<false>(
            violation_base, &data->location, "%s address %p, Type: %s", type_check_kind_str,
            reinterpret_cast<void*>(ptr), data->type->name
        );
    }
}
UBSAN_ABORT_VARIANT(type_mismatch_v1, UbsanTypeMismatchData*, data, uintptr_t, ptr)

// --- Alignment Assumption ---
DECL_C void __ubsan_handle_alignment_assumption(
    UbsanAlignmentAssumptionData* data, uintptr_t ptr, uintptr_t alignment, uintptr_t offset
)
{
    UbsanReport<false>(
        "Alignment assumption failed", &data->location,
        "Pointer: %p, Alignment: %lu, Offset: %lu, Type: '%s'", reinterpret_cast<void*>(ptr),
        alignment, offset, data->type->name
    );
}
UBSAN_ABORT_VARIANT(
    alignment_assumption, UbsanAlignmentAssumptionData*, data, uintptr_t, ptr, uintptr_t, alignment,
    uintptr_t, offset
)

// --- Add Overflow ---
DECL_C void __ubsan_handle_add_overflow(UbsanOverflowData* data, uintptr_t lhs, uintptr_t rhs)
{
    UbsanReport<false>(
        "Addition overflow", &data->location, "Type: '%s', LHS: %#lx, RHS: %#lx", data->type->name,
        lhs, rhs
    );
}
UBSAN_ABORT_VARIANT(add_overflow, UbsanOverflowData*, data, uintptr_t, lhs, uintptr_t, rhs)

// --- Sub Overflow ---R
DECL_C void __ubsan_handle_sub_overflow(UbsanOverflowData* data, uintptr_t lhs, uintptr_t rhs)
{
    UbsanReport<false>(
        "Subtraction overflow", &data->location, "Type: '%s', LHS: %#lx, RHS: %#lx",
        data->type->name, lhs, rhs
    );
}
UBSAN_ABORT_VARIANT(sub_overflow, UbsanOverflowData*, data, uintptr_t, lhs, uintptr_t, rhs)

// --- Mul Overflow ---
DECL_C void __ubsan_handle_mul_overflow(UbsanOverflowData* data, uintptr_t lhs, uintptr_t rhs)
{
    UbsanReport<false>(
        "Multiplication overflow", &data->location, "Type: '%s', LHS: %#lx, RHS: %#lx",
        data->type->name, lhs, rhs
    );
}
UBSAN_ABORT_VARIANT(mul_overflow, UbsanOverflowData*, data, uintptr_t, lhs, uintptr_t, rhs)

// --- Negate Overflow ---
DECL_C void __ubsan_handle_negate_overflow(UbsanOverflowData* data, uintptr_t old_val)
{
    UbsanReport<false>(
        "Negation overflow", &data->location, "Type: '%s', Value: %#lx", data->type->name, old_val
    );
}
UBSAN_ABORT_VARIANT(negate_overflow, UbsanOverflowData*, data, uintptr_t, old_val)

// --- Divrem Overflow ---
DECL_C void __ubsan_handle_divrem_overflow(UbsanOverflowData* data, uintptr_t lhs, uintptr_t rhs)
{
    const char* violation = (rhs == 0) ? "Division by zero" : "Division overflow";
    UbsanReport<false>(
        violation, &data->location, "Type: '%s', LHS: %#lx, RHS: %#lx", data->type->name, lhs, rhs
    );
}
UBSAN_ABORT_VARIANT(divrem_overflow, UbsanOverflowData*, data, uintptr_t, lhs, uintptr_t, rhs)

// --- Shift Out Of Bounds ---
DECL_C void __ubsan_handle_shift_out_of_bounds(
    UbsanShiftOutOfBoundsData* data, uintptr_t lhs, uintptr_t rhs
)
{
    UbsanReport<false>(
        "Shift out of bounds", &data->location,
        "LHS Type: '%s', RHS Type: '%s', LHS: %#lx, RHS: %#lx", data->lhs_type->name,
        data->rhs_type->name, lhs, rhs
    );
}
UBSAN_ABORT_VARIANT(
    shift_out_of_bounds, UbsanShiftOutOfBoundsData*, data, uintptr_t, lhs, uintptr_t, rhs
)

// --- Out Of Bounds ---
DECL_C void __ubsan_handle_out_of_bounds(UbsanOutOfBoundsData* data, uintptr_t index)
{
    UbsanReport<false>(
        "Out of bounds access", &data->location, "Index: %#lx, Left Type: '%s', Right Type: '%s'",
        index, data->left_type->name, data->right_type->name
    );
}
UBSAN_ABORT_VARIANT(out_of_bounds, UbsanOutOfBoundsData*, data, uintptr_t, index)

// --- Builtin Unreachable ---
DECL_C NO_RET void __ubsan_handle_builtin_unreachable(UbsanUnreachableData* data)
{
    UbsanReport<true>("Execution reached an unreachable program point", &data->location);
}

// --- Missing Return ---
DECL_C NO_RET void __ubsan_handle_missing_return(UbsanUnreachableData* data)
{
    UbsanReport<true>(
        "Execution reached the end of a value-returning function without returning a value",
        &data->location
    );
}

// --- VLA Bound Not Positive ---
DECL_C void __ubsan_handle_vla_bound_not_positive(UbsanVLABoundData* data, uintptr_t bound)
{
    UbsanReport<false>(
        "Variable-length array bound not positive", &data->location, "Bound: %lu, Type: '%s'",
        bound, data->type->name
    );
}
UBSAN_ABORT_VARIANT(vla_bound_not_positive, UbsanVLABoundData*, data, uintptr_t, bound)

// --- Float Cast Overflow ---
DECL_C void __ubsan_handle_float_cast_overflow(UbsanFloatCastOverflowData* data, uintptr_t from)
{
    UbsanReport<false>(
        "Floating-point cast overflow", &data->location,
        "From Type: '%s', To Type: '%s', Value Bits: %#lx", data->from_type->name,
        data->to_type->name, from
    );
}
UBSAN_ABORT_VARIANT(float_cast_overflow, UbsanFloatCastOverflowData*, data, uintptr_t, from)

// --- Load Invalid Value ---
DECL_C void __ubsan_handle_load_invalid_value(UbsanInvalidValueData* data, uintptr_t val)
{
    UbsanReport<false>(
        "Load of invalid value", &data->location, "Type: '%s', Value: %#lx", data->type->name, val
    );
}
UBSAN_ABORT_VARIANT(load_invalid_value, UbsanInvalidValueData*, data, uintptr_t, val)

// --- Implicit Conversion ---
DECL_C void __ubsan_handle_implicit_conversion(
    UbsanImplicitConversionData* data, uintptr_t src, uintptr_t dst
)
{
    const char* kKinds[] = {
        "integer truncation", "unsigned integer truncation", "signed integer truncation",
        "integer sign change", "signed integer truncation or sign change"
    };
    const char* kind_str = data->kind < ARRAY_SIZE(kKinds) ? kKinds[data->kind] : "unknown kind";
    UbsanReport<false>(
        "Implicit conversion issue", &data->location,
        "Kind: '%s', From Type: '%s', To Type: '%s', Src Value: %#lx, Dst Value: %#lx", kind_str,
        data->from_type->name, data->to_type->name, src, dst
    );
}
UBSAN_ABORT_VARIANT(
    implicit_conversion, UbsanImplicitConversionData*, data, uintptr_t, src, uintptr_t, dst
)

// --- Invalid Builtin ---
DECL_C void __ubsan_handle_invalid_builtin(UbsanInvalidBuiltinData* data)
{
    const char* kind_str = data->kind < ARRAY_SIZE(kBuiltinCheckKinds)
                               ? kBuiltinCheckKinds[data->kind]
                               : "unknown kind";
    UbsanReport<false>(
        "Invalid builtin function usage", &data->location, "Kind: '%s' (%hhu)", kind_str, data->kind
    );
}
UBSAN_ABORT_VARIANT(invalid_builtin, UbsanInvalidBuiltinData*, data)

// --- Invalid ObjC Cast ---
DECL_C void __ubsan_handle_invalid_objc_cast(UbsanInvalidObjCCastData* data, uintptr_t ptr)
{
    UbsanReport<false>(
        "Invalid Objective-C cast", &data->location, "Pointer: %p, Expected Type: '%s'",
        reinterpret_cast<void*>(ptr), data->expected_type->name
    );
}
UBSAN_ABORT_VARIANT(invalid_objc_cast, UbsanInvalidObjCCastData*, data, uintptr_t, ptr)

// --- Nonnull Return v1 ---
DECL_C void __ubsan_handle_nonnull_return_v1(
    UbsanNonNullReturnData* data, UbsanSourceLocation* loc_ptr
)
{
    UbsanReport<false>(
        "Null return from function declared to never return null",
        loc_ptr ? loc_ptr : &data->attr_location
    );
}
UBSAN_ABORT_VARIANT(nonnull_return_v1, UbsanNonNullReturnData*, data, UbsanSourceLocation*, loc_ptr)

// --- Nullability Return v1 ---
DECL_C void __ubsan_handle_nullability_return_v1(
    UbsanNonNullReturnData* data, UbsanSourceLocation* loc_ptr
)
{
    UbsanReport<false>(
        "Null return from function with non-null return annotation",
        loc_ptr ? loc_ptr : &data->attr_location
    );
}
UBSAN_ABORT_VARIANT(
    nullability_return_v1, UbsanNonNullReturnData*, data, UbsanSourceLocation*, loc_ptr
)

// --- Nonnull Arg ---
DECL_C void __ubsan_handle_nonnull_arg(UbsanNonNullArgData* data)
{
    UbsanReport<false>(
        "Null pointer passed as argument", &data->location,
        "Argument Index: %d, Declared non-null at %s:%u:%u", data->arg_index,
        data->attr_location.file, data->attr_location.line, data->attr_location.column
    );
}
UBSAN_ABORT_VARIANT(nonnull_arg, UbsanNonNullArgData*, data)

// --- Nullability Arg ---
DECL_C void __ubsan_handle_nullability_arg(UbsanNonNullArgData* data)
{
    UbsanReport<false>(
        "Null pointer passed as argument", &data->location,
        "Argument Index: %d, Annotated non-null at %s:%u:%u", data->arg_index,
        data->attr_location.file, data->attr_location.line, data->attr_location.column
    );
}
UBSAN_ABORT_VARIANT(nullability_arg, UbsanNonNullArgData*, data)

// --- Pointer Overflow ---
DECL_C void __ubsan_handle_pointer_overflow(
    UbsanPointerOverflowData* data, uintptr_t base, uintptr_t result
)
{
    UbsanReport<false>(
        "Pointer arithmetic overflow", &data->location, "Base: %p, Result: %p",
        reinterpret_cast<void*>(base), reinterpret_cast<void*>(result)
    );
}
UBSAN_ABORT_VARIANT(
    pointer_overflow, UbsanPointerOverflowData*, data, uintptr_t, base, uintptr_t, result
)

// --- CFI Bad ICall ---
DECL_C void __ubsan_handle_cfi_bad_icall(UbsanCFIBadIcallData* data, uintptr_t function)
{
    UbsanReport<false>(
        "Control Flow Integrity: bad indirect function call", &data->location,
        "Target: %p, Expected Type: '%s'", reinterpret_cast<void*>(function), data->type->name
    );
}
UBSAN_ABORT_VARIANT(cfi_bad_icall, UbsanCFIBadIcallData*, data, uintptr_t, function)

// --- CFI Check Fail ---
DECL_C void __ubsan_handle_cfi_check_fail(
    UbsanCFICheckFailData* data, uintptr_t function, uintptr_t vtable_is_valid
)
{
    const char* kind_str = data->check_kind < ARRAY_SIZE(kCFITypeCheckKinds)
                               ? kCFITypeCheckKinds[data->check_kind]
                               : "unknown kind";
    UbsanReport<false>(
        "Control Flow Integrity check failure", &data->location,
        "Check Kind: '%s' (%hhu), Target: %p, VTable Valid: %lu", kind_str, data->check_kind,
        reinterpret_cast<void*>(function), vtable_is_valid
    );
}
UBSAN_ABORT_VARIANT(
    cfi_check_fail, UbsanCFICheckFailData*, data, uintptr_t, function, uintptr_t, vtable_is_valid
)

// --- CFI Bad Type ---
DECL_C void __ubsan_handle_cfi_bad_type(
    UbsanCFICheckFailData* data, uintptr_t vtable, bool valid_vtable, UbsanReportOptions opts
)
{
    UbsanReport<false>(
        "Control Flow Integrity: bad type", &data->location,
        "VTable Ptr: %p, Valid VTable: %s, PC: %#lx, BP: %#lx", reinterpret_cast<void*>(vtable),
        valid_vtable ? "true" : "false", opts.pc, opts.bp
    );
}
UBSAN_ABORT_VARIANT(
    cfi_bad_type, UbsanCFICheckFailData*, data, uintptr_t, vtable, bool, valid_vtable,
    UbsanReportOptions, opts
)

// --- Function Type Mismatch ---
DECL_C void __ubsan_handle_function_type_mismatch(
    UbsanFunctionTypeMismatchData* data, uintptr_t val
)
{
    UbsanReport<false>(
        "Function called through pointer with incompatible type", &data->location,
        "Function Pointer: %p, Required Type: '%s'", reinterpret_cast<void*>(val), data->type->name
    );
}
UBSAN_ABORT_VARIANT(function_type_mismatch, UbsanFunctionTypeMismatchData*, data, uintptr_t, val)
