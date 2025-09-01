# Project Logging Contract

This document is the logging authority. The core mandate is: **Logging occurs exclusively at call boundaries where a function's `std::expected` result forces a change in control flow. The default is silence.**

---

### 1. The Worker Function Contract

-   **Thou Shalt Not Log Thine Own Outcome:** Functions that perform an operation **must not** log their own success or failure. Their sole responsibility is to execute and report status via their return value.
-   **The `std::expected` Mandate:** Any function that can fail **must** return `std::expected<Value, Error>`. Actions that can fail but return no value **must** return `std::expected<void, Error>`.

This contract renders worker functions pure; they are decoupled from the diagnostic system.

```cpp
// COMPLIANT: This function is a pure worker. It is silent.
std::expected<Pml4Table*, MMapError> CreateAddressSpace() {
    auto pml4 = AllocatePage();
    if (!pml4) {
        return std::unexpected(MMapError::OutOfMemory);
    }
    // ... further setup ...
    return pml4;
}

// NON-COMPLIANT: This function violates the contract by logging its own outcome.
std::expected<Pml4Table*, MMapError> CreateAddressSpace() {
    LOG_DEBUG("Attempting to allocate PML4 table..."); // VIOLATION
    // ...
    if (!pml4) {
        LOG_ERROR("PML4 allocation failed!"); // VIOLATION
        return std::unexpected(MMapError::OutOfMemory);
    }
    LOG_INFO("PML4 allocated successfully."); // VIOLATION
    return pml4;
}
```

---

### 2. The Caller's Contract: The Boundary of Consequence

All logging is performed by the caller when it unpacks an `std::expected` result. This is the **Boundary of Consequence**, the point where a decision must be made.

-   **On Failure (`!result`):** The system's path is altered. The failure **must** be logged.
-   **On Success (`result`):**
    -   **Release Build (`MAX_LOG_LEVEL <= INFO`):** No log is generated. Silence implies success.
    -   **Debug Build (`MAX_LOG_LEVEL >= DEBUG`):** A `DEBUG` level log is used to instrument the new state (e.g., log the returned value, address, or handle).

---

### 3. The "Major Milestone" Exception

The only exception to "Silent Success" is the logging of a **Major Milestone**.

-   **Definition:** An irreversible, significant system state transition, such as a change in CPU operating mode or the activation of a core service. These are chapter headings of the boot process, not descriptions of intermediate steps.
-   **Implementation:** An explicit `INFO` level logging call made by the caller *after* a sequence of operations has successfully completed and established the new state.

**Valid Milestones:**
- `LOG_INFO("Paging enabled. Entering higher half.");`
- `LOG_INFO("Scheduler started. Relinquishing direct control.");`

**Invalid (Violates Silence):**
- `LOG_INFO("Found Multiboot memory map tag.");`
- `LOG_INFO("ELF sections loaded successfully.");`

---

### 4. Decision Matrix & Example

| Outcome | Caller Action (Release Build) | Caller Action (Debug Build) |
| :--- | :--- | :--- |
| **Standard Success** | **No action.** Continue execution. | `LOG_DEBUG("Address space created at {:#x}", result->pml4);` |
| **Major Milestone Achieved** | `LOG_INFO("Long mode enabled.");` | `LOG_INFO("Long mode enabled.");` |
| **Recoverable Failure** | `LOG_WARN("ACPI parse failed: {}. Using BIOS.", result.error());` | `LOG_WARN("ACPI parse failed: {}. Using BIOS.", result.error());` |
| **Fatal Failure** | `LOG_ERROR("Failed to map kernel: {}. Halting.", result.error()); SYSTEM_PANIC(...)` | `LOG_ERROR("Failed to map kernel: {}. Halting.", result.error()); SYSTEM_PANIC(...)` |

**Code Example (Caller):**
```cpp
// This is the Boundary of Consequence. Logging happens here, and only here.
auto result = CreateAddressSpace(); 
if (!result) {
    // Failure path: Always log.
    LOG_ERROR("Could not create initial address space: {}", to_string(result.error()));
    SYSTEM_PANIC("System memory initialization failed.");
}

// Success path:
// Release build is silent. Debug build instruments the state.
LOG_DEBUG("Initial PML4 table allocated at physical address {:#x}", *result);

// Now, we enable paging (a major milestone).
EnablePaging(*result);
LOG_INFO("Paging enabled. CPU virtual memory is now active."); // Milestone log.
```
