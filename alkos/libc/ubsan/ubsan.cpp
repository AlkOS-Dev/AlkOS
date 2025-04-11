#include "assert.h"
#include "extensions/bit.hpp"
#include "extensions/debug.hpp"
#include "extensions/defines.hpp"
#include "extensions/internal/macro.hpp"
#include "extensions/utility.hpp"
#include "stdarg.h"

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
#if !(defined(__GNUC__) && __GNUC__ < 6)
    UbsanSourceLocation location;
#endif
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

static int vFormatUbsanMessage(
    char* buffer, const size_t buffer_size, const char* violation_type,
    const UbsanSourceLocation* location, const char* format, va_list args
)
{
    if (buffer == nullptr || buffer_size == 0) {
        return 0;
    }

    if (location == nullptr) {
        location = &kUnknownLocation;
    }

    int offset = 0;

    // Format violation type and location information
    offset += snprintf(
        buffer + offset, buffer_size - offset, "UBSAN: %s at %s:%u:%u", violation_type,
        location->file, location->line, location->column
    );

    // If we have additional format and arguments, add them to the message
    if (format && static_cast<size_t>(offset) < buffer_size) {
        offset += snprintf(buffer + offset, buffer_size - offset, ":\n");
        offset += vsnprintf(buffer + offset, buffer_size - offset, format, args);
    }

    return offset;
}

template <bool fatal>
static void UbsanReport(
    const char* violation_type, const UbsanSourceLocation* location, const char* format = nullptr,
    ...
)
{
    constexpr u64 kBufferSize = 512;
    char buffer[kBufferSize];

    va_list args;
    va_start(args, format);
    vFormatUbsanMessage(buffer, sizeof(buffer), violation_type, location, format, args);
    va_end(args);

    if constexpr (fatal) {
        if constexpr (kIsKernel) {
            KernelPanic(buffer);
        } else {
            TODO_USERSPACE
            // TODO(F1r3d3v): Implement user space panic/process termination
        }

        std::unreachable();
    } else {
        TODO_LIBK_TRACE
        // TODO(F1r3d3v): Enable tracing when using libk
        TRACE_WARNING(buffer);
    }
}

////////////////////////////
//     Implementation     //
////////////////////////////

#define GET_FIRST_PARAM_NAME(x, y, ...) y
#define GET_SECOND(x, y)                y
#define MERGE(x, y)                     x y
#define FOR_EACH_MERGE(...)             FOR_EACH_PAIR(MERGE, __VA_ARGS__)
#define FOR_EACH_GET_SECOND(...)        FOR_EACH_PAIR(GET_SECOND, __VA_ARGS__)

#define UBSAN_RECOVERABLE(handler_name, check_name, ...)                                \
    DECL_C void __ubsan_handle_##check_name(FOR_EACH_MERGE(__VA_ARGS__))                \
    {                                                                                   \
        ASSERT_NOT_NULL(GET_FIRST_PARAM_NAME(__VA_ARGS__));                             \
        handler_name<false>(FOR_EACH_GET_SECOND(__VA_ARGS__));                          \
    }                                                                                   \
                                                                                        \
    DECL_C NO_RET void __ubsan_handle_##check_name##_abort(FOR_EACH_MERGE(__VA_ARGS__)) \
    {                                                                                   \
        ASSERT_NOT_NULL(GET_FIRST_PARAM_NAME(__VA_ARGS__));                             \
        handler_name<true>(FOR_EACH_GET_SECOND(__VA_ARGS__));                           \
        std::unreachable();                                                             \
    }

#define UBSAN_UNRECOVERABLE(handler_name, check_name, ...)                      \
    DECL_C NO_RET void __ubsan_handle_##check_name(FOR_EACH_MERGE(__VA_ARGS__)) \
    {                                                                           \
        ASSERT_NOT_NULL(GET_FIRST_PARAM_NAME(__VA_ARGS__));                     \
        handler_name<true>(FOR_EACH_GET_SECOND(__VA_ARGS__));                   \
        std::unreachable();                                                     \
    }

// --- Type Mismatch ---
template <bool fatal>
void HandleTypeMismatch(UbsanTypeMismatchData* data, uintptr_t ptr)
{
    constexpr const char* kTypeCheckKinds[] = {
        "Load operation on",     "Store operation to",      "Reference binding to",
        "Member access within",  "Member function call on", "Constructor invocation on",
        "Downcast operation on", "Secondary downcast on",   "Upcast operation on",
        "Virtual base cast on",  "Non-null binding to",     "Dynamic operation on"
    };

    const char* type_check_kind_str = data->type_check_kind < ARRAY_SIZE(kTypeCheckKinds)
                                          ? kTypeCheckKinds[data->type_check_kind]
                                          : "Unknown";

    if (ptr == 0) {
        UbsanReport<fatal>(
            "Null pointer access", &data->location, "%s null pointer of type %s",
            type_check_kind_str, data->type->name
        );
    } else if (data->alignment != 0 && !IsAligned(ptr, data->alignment)) {
        UbsanReport<fatal>(
            "Misaligned memory access", &data->location,
            "%s misaligned address %p for type %s\n"
            "which requires %u byte alignment",
            type_check_kind_str, reinterpret_cast<void*>(ptr), data->type->name, data->alignment
        );
    } else {
        UbsanReport<fatal>(
            "Object size mismatch", &data->location,
            "%s address %p with insufficient space for an object of type %s", type_check_kind_str,
            reinterpret_cast<void*>(ptr), data->type->name
        );
    }
}
UBSAN_RECOVERABLE(
    HandleTypeMismatch, type_mismatch_v1, UbsanTypeMismatchData*, data, uintptr_t, ptr
)

// --- Alignment Assumption ---
template <bool fatal>
void HandleAlignmentAssumption(
    UbsanAlignmentAssumptionData* data, uintptr_t ptr, uintptr_t alignment, uintptr_t offset
)
{
    UbsanReport<fatal>(
        "Alignment assumption failed", &data->location,
        "Assumption of %lu byte alignment for pointer %p with offset %lu failed\n"
        "Type: '%s'",
        alignment, reinterpret_cast<void*>(ptr), offset, data->type->name
    );
}
UBSAN_RECOVERABLE(
    HandleAlignmentAssumption, alignment_assumption, UbsanAlignmentAssumptionData*, data, uintptr_t,
    ptr, uintptr_t, alignment, uintptr_t, offset
)

// --- Add Overflow ---
template <bool fatal>
void HandleArithmeticOverflow(UbsanOverflowData* data, uintptr_t lhs, uintptr_t rhs, char op)
{
    const char* violation_type;

    switch (op) {
        case '+':
            violation_type = "Addition overflow";
            break;
        case '-':
            violation_type = "Subtraction overflow";
            break;
        case '*':
            violation_type = "Multiplication overflow";
            break;
        default:
            violation_type = "Unknown arithmetic operation";
    }

    UbsanReport<fatal>(
        violation_type, &data->location,
        "%s integer overflow:\n"
        "%#lx %c %#lx cannot be represented in type %s",
        data->type->info & 1 ? "Signed" : "Unsigned", lhs, op, rhs, data->type->name
    );
}

// --- Add Overflow ---
template <bool fatal>
WRAP_CALL void HandleAddOverflow(UbsanOverflowData* data, uintptr_t lhs, uintptr_t rhs)
{
    HandleArithmeticOverflow<fatal>(data, lhs, rhs, '+');
}
UBSAN_RECOVERABLE(
    HandleAddOverflow, add_overflow, UbsanOverflowData*, data, uintptr_t, lhs, uintptr_t, rhs
)

// --- Sub Overflow ---
template <bool fatal>
WRAP_CALL void HandleSubOverflow(UbsanOverflowData* data, uintptr_t lhs, uintptr_t rhs)
{
    HandleArithmeticOverflow<fatal>(data, lhs, rhs, '-');
}
UBSAN_RECOVERABLE(
    HandleSubOverflow, sub_overflow, UbsanOverflowData*, data, uintptr_t, lhs, uintptr_t, rhs
)

// --- Mul Overflow ---
template <bool fatal>
WRAP_CALL void HandleMulOverflow(UbsanOverflowData* data, uintptr_t lhs, uintptr_t rhs)
{
    HandleArithmeticOverflow<fatal>(data, lhs, rhs, '*');
}
UBSAN_RECOVERABLE(
    HandleMulOverflow, mul_overflow, UbsanOverflowData*, data, uintptr_t, lhs, uintptr_t, rhs
)

// --- Negate Overflow ---
template <bool fatal>
void HandleNegateOverflow(UbsanOverflowData* data, uintptr_t old_val)
{
    UbsanReport<fatal>(
        "Negation overflow", &data->location, "negation of %#lx cannot be represented in type %s",
        old_val, data->type->name
    );
}
UBSAN_RECOVERABLE(
    HandleNegateOverflow, negate_overflow, UbsanOverflowData*, data, uintptr_t, old_val
)

// --- Divrem Overflow ---
template <bool fatal>
void HandleDivremOverflow(UbsanOverflowData* data, uintptr_t lhs, uintptr_t rhs)
{
    if (rhs == 0) {
        UbsanReport<fatal>("Division by zero", &data->location, "Type: '%s'", data->type->name);
    } else if (data->type->info & 1 && rhs == static_cast<uintptr_t>(-1) &&
               lhs == static_cast<uintptr_t>(-1ull >> 1) + 1) {
        UbsanReport<fatal>(
            "Division overflow", &data->location,
            "division of %#lx by -1 cannot be represented in type %s", lhs, data->type->name
        );
    } else {
        UbsanReport<fatal>("Division error", &data->location, "Type: '%s'", data->type->name);
    }
}
UBSAN_RECOVERABLE(
    HandleDivremOverflow, divrem_overflow, UbsanOverflowData*, data, uintptr_t, lhs, uintptr_t, rhs
)

// --- Shift Out Of Bounds ---
template <bool fatal>
void HandleShiftOutOfBounds(UbsanShiftOutOfBoundsData* data, uintptr_t lhs, uintptr_t rhs)
{
    bool is_lhs_signed = data->lhs_type->info & 1;
    bool is_rhs_signed = data->rhs_type->info & 1;
    u64 lhs_bit_width  = 1 << (data->lhs_type->info >> 1);

    if (is_rhs_signed && static_cast<intptr_t>(rhs) < 0) {
        UbsanReport<fatal>(
            "Shift out of bounds", &data->location, "shift exponent %#lx is negative", rhs
        );
    } else if (rhs >= lhs_bit_width) {
        UbsanReport<fatal>(
            "Shift out of bounds", &data->location,
            "shift exponent %#lx is too large for %u-bit type %s", rhs, lhs_bit_width,
            data->lhs_type->name
        );
    } else if (is_lhs_signed && static_cast<intptr_t>(lhs) < 0) {
        UbsanReport<fatal>(
            "Shift out of bounds", &data->location, "left shift of negative value %#lx", lhs
        );
    } else {
        UbsanReport<fatal>(
            "Shift out of bounds", &data->location,
            "left shift of %#lx by %#lx places cannot be represented in type %s", lhs, rhs,
            data->lhs_type->name
        );
    }
}
UBSAN_RECOVERABLE(
    HandleShiftOutOfBounds, shift_out_of_bounds, UbsanShiftOutOfBoundsData*, data, uintptr_t, lhs,
    uintptr_t, rhs
)

// --- Out Of Bounds ---
template <bool fatal>
void HandleOutOfBounds(UbsanOutOfBoundsData* data, uintptr_t index)
{
    UbsanReport<fatal>(
        "Index out of bounds", &data->location, "index %#lx is out of range for type %s", index,
        data->right_type->name
    );
}
UBSAN_RECOVERABLE(HandleOutOfBounds, out_of_bounds, UbsanOutOfBoundsData*, data, uintptr_t, index)

// --- Builtin Unreachable ---
template <bool fatal>
void BuiltinUnreachable(UbsanUnreachableData* data)
{
    UbsanReport<fatal>("Execution reached an unreachable program point", &data->location);
}
UBSAN_UNRECOVERABLE(BuiltinUnreachable, builtin_unreachable, UbsanUnreachableData*, data)

// --- Missing Return ---
template <bool fatal>
void MissingReturn(UbsanUnreachableData* data)
{
    UbsanReport<fatal>(
        "Execution reached the end of a value-returning function without returning a value",
        &data->location
    );
}
UBSAN_UNRECOVERABLE(MissingReturn, missing_return, UbsanUnreachableData*, data)

// --- VLA Bound Not Positive ---
template <bool fatal>
void HandleVLABoundNotPositive(UbsanVLABoundData* data, uintptr_t bound)
{
    UbsanReport<fatal>(
        "Variable-length array bound not positive", &data->location,
        "variable length array bound value %lu <= 0", bound
    );
}
UBSAN_RECOVERABLE(
    HandleVLABoundNotPositive, vla_bound_not_positive, UbsanVLABoundData*, data, uintptr_t, bound
)

// --- Float Cast Overflow ---
template <bool fatal>
void HandleFloatCastOverflow(UbsanFloatCastOverflowData* data, uintptr_t from)
{
#if !(defined(__GNUC__) && __GNUC__ < 6)
    UbsanReport<fatal>(
        "Floating-point cast overflow", &data->location,
        "value %#lx from type '%s' cannot be represented in type '%s'", from, data->from_type->name,
        data->to_type->name
    );
#else
    UbsanReport<fatal>(
        "Floating-point cast overflow", nullptr_t,
        "value %#lx from type '%s' cannot be represented in type '%s'", from, data->from_type->name,
        data->to_type->name
    );
#endif
}
UBSAN_RECOVERABLE(
    HandleFloatCastOverflow, float_cast_overflow, UbsanFloatCastOverflowData*, data, uintptr_t, from
)

// --- Load Invalid Value ---
template <bool fatal>
void HandleLoadInvalidValue(UbsanInvalidValueData* data, uintptr_t val)
{
    UbsanReport<fatal>(
        "Load of invalid value", &data->location,
        "load of value %#lx is not a valid value for type '%s'", val, data->type->name
    );
}
UBSAN_RECOVERABLE(
    HandleLoadInvalidValue, load_invalid_value, UbsanInvalidValueData*, data, uintptr_t, val
)

// --- Implicit Conversion ---
template <bool fatal>
void HandleImplicitConversion(UbsanImplicitConversionData* data, uintptr_t src, uintptr_t dst)
{
    constexpr const char* kKinds[] = {
        "Integer truncation", "Unsigned integer truncation", "Signed integer truncation",
        "Integer sign change", "Signed integer truncation or sign change"
    };

    const char* kind_str = data->kind < ARRAY_SIZE(kKinds) ? kKinds[data->kind] : "Unknown";

    UbsanReport<fatal>(
        "Implicit conversion issue", &data->location,
        "Kind: '%s', From Type: '%s', To Type: '%s', Src Value: %#lx, Dst Value: %#lx", kind_str,
        data->from_type->name, data->to_type->name, src, dst
    );
}
UBSAN_RECOVERABLE(
    HandleImplicitConversion, implicit_conversion, UbsanImplicitConversionData*, data, uintptr_t,
    src, uintptr_t, dst
)

// --- Invalid Builtin ---
template <bool fatal>
void HandleInvalidBuiltin(UbsanInvalidBuiltinData* data)
{
    constexpr const char* kBuiltinCheckKinds[] = {
        "Count trailing zeros of zero", "Count leading zeros of zero", "Assume condition false"
    };

    const char* kind_str =
        data->kind < ARRAY_SIZE(kBuiltinCheckKinds) ? kBuiltinCheckKinds[data->kind] : "Unknown";

    UbsanReport<fatal>(
        "Invalid builtin function usage", &data->location, "Kind: '%s' (%hhu)", kind_str, data->kind
    );
}
UBSAN_RECOVERABLE(HandleInvalidBuiltin, invalid_builtin, UbsanInvalidBuiltinData*, data)

// --- Invalid ObjC Cast ---
template <bool fatal>
void HandleInvalidObjCCast(UbsanInvalidObjCCastData* data, uintptr_t ptr)
{
    UbsanReport<fatal>(
        "Invalid Objective-C cast", &data->location, "Pointer: %p, Expected Type: '%s'",
        reinterpret_cast<void*>(ptr), data->expected_type->name
    );
}
UBSAN_RECOVERABLE(
    HandleInvalidObjCCast, invalid_objc_cast, UbsanInvalidObjCCastData*, data, uintptr_t, ptr
)

// --- Nonnull Return v1 ---
template <bool fatal>
void HandleNonnullReturnV1(UbsanNonNullReturnData* data, UbsanSourceLocation* loc_ptr)
{
    UbsanReport<fatal>(
        "Null return from function declared to never return null",
        loc_ptr ? loc_ptr : &data->attr_location
    );
}
UBSAN_RECOVERABLE(
    HandleNonnullReturnV1, nonnull_return_v1, UbsanNonNullReturnData*, data, UbsanSourceLocation*,
    loc_ptr
)

// --- Nullability Return v1 ---
template <bool fatal>
void HandleNullabilityReturnV1(UbsanNonNullReturnData* data, UbsanSourceLocation* loc_ptr)
{
    UbsanReport<fatal>(
        "Null return from function with non-null return annotation",
        loc_ptr ? loc_ptr : &data->attr_location
    );
}
UBSAN_RECOVERABLE(
    HandleNullabilityReturnV1, nullability_return_v1, UbsanNonNullReturnData*, data,
    UbsanSourceLocation*, loc_ptr
)

// --- Nonnull Arg ---
template <bool fatal>
void HandleNonnullArg(UbsanNonNullArgData* data)
{
    UbsanReport<fatal>(
        "Null pointer passed as argument", &data->location,
        "Argument Index: %d, Declared non-null at %s:%u:%u", data->arg_index,
        data->attr_location.file, data->attr_location.line, data->attr_location.column
    );
}
UBSAN_RECOVERABLE(HandleNonnullArg, nonnull_arg, UbsanNonNullArgData*, data)

// --- Nullability Arg ---
template <bool fatal>
void HandleNullabilityArg(UbsanNonNullArgData* data)
{
    UbsanReport<fatal>(
        "Null pointer passed as argument", &data->location,
        "Argument Index: %d, Annotated non-null at %s:%u:%u", data->arg_index,
        data->attr_location.file, data->attr_location.line, data->attr_location.column
    );
}
UBSAN_RECOVERABLE(HandleNullabilityArg, nullability_arg, UbsanNonNullArgData*, data)

// --- Pointer Overflow ---
template <bool fatal>
void HandlePointerOverflow(UbsanPointerOverflowData* data, uintptr_t base, uintptr_t result)
{
    UbsanReport<fatal>(
        "Pointer arithmetic overflow", &data->location, "Base: %p, Result: %p",
        reinterpret_cast<void*>(base), reinterpret_cast<void*>(result)
    );
}
UBSAN_RECOVERABLE(
    HandlePointerOverflow, pointer_overflow, UbsanPointerOverflowData*, data, uintptr_t, base,
    uintptr_t, result
)

// --- CFI Bad ICall ---
template <bool fatal>
void HandleCFIBadICall(UbsanCFIBadIcallData* data, uintptr_t function)
{
    UbsanReport<fatal>(
        "Control Flow Integrity: bad indirect function call", &data->location,
        "Target: %p, Expected Type: '%s'", reinterpret_cast<void*>(function), data->type->name
    );
}
UBSAN_RECOVERABLE(
    HandleCFIBadICall, cfi_bad_icall, UbsanCFIBadIcallData*, data, uintptr_t, function
)

// --- CFI Check Fail ---
template <bool fatal>
void HandleCFICheckFail(UbsanCFICheckFailData* data, uintptr_t function, uintptr_t vtable_is_valid)
{
    constexpr const char* kCFITypeCheckKinds[] = {
        "Virtual call",
        "Non-virtual call",
        "Derived cast",
        "Unrelated cast",
        "Indirect call",
        "Non-virtual member function call",
        "Virtual member function call"
    };

    const char* kind_str = data->check_kind < ARRAY_SIZE(kCFITypeCheckKinds)
                               ? kCFITypeCheckKinds[data->check_kind]
                               : "Unknown";
    UbsanReport<fatal>(
        "Control Flow Integrity check failure", &data->location,
        "Check Kind: '%s' (%hhu), Target: %p, VTable Valid: %lu", kind_str, data->check_kind,
        reinterpret_cast<void*>(function), vtable_is_valid
    );
}
UBSAN_RECOVERABLE(
    HandleCFICheckFail, cfi_check_fail, UbsanCFICheckFailData*, data, uintptr_t, function,
    uintptr_t, vtable_is_valid
)

// --- CFI Bad Type ---
template <bool fatal>
void HandleCFIBadType(
    UbsanCFICheckFailData* data, uintptr_t vtable, bool valid_vtable, UbsanReportOptions opts
)
{
    UbsanReport<fatal>(
        "Control Flow Integrity: bad type", &data->location,
        "VTable Ptr: %p, Valid VTable: %s, PC: %#lx, BP: %#lx", reinterpret_cast<void*>(vtable),
        valid_vtable ? "true" : "false", opts.pc, opts.bp
    );
}
UBSAN_RECOVERABLE(
    HandleCFIBadType, cfi_bad_type, UbsanCFICheckFailData*, data, uintptr_t, vtable, bool,
    valid_vtable, UbsanReportOptions, opts
)

// --- Function Type Mismatch ---
template <bool fatal>
void HandleFunctionTypeMismatch(UbsanFunctionTypeMismatchData* data, uintptr_t val)
{
    UbsanReport<fatal>(
        "Function called through pointer with incompatible type", &data->location,
        "Function Pointer: %p, Required Type: '%s'", reinterpret_cast<void*>(val), data->type->name
    );
}
UBSAN_RECOVERABLE(
    HandleFunctionTypeMismatch, function_type_mismatch, UbsanFunctionTypeMismatchData*, data,
    uintptr_t, val
)
