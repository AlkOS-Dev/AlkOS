# AlkOS Feature Flags System

The AlkOS feature flags system provides a flexible way to enable or disable kernel features at build time. This system uses a declarative YAML configuration that generates both C/C++ headers and build system configurations.

## For Developers: Understanding the Feature Flags Architecture

### Why YAML Configuration?

The feature flags system uses YAML (`feature_flags_defs.yaml`) as the single source of truth for several key benefits:

- **Declarative Configuration**: All feature flags, their descriptions, and default values are defined in one readable location
- **Type Safety**: YAML structure ensures consistent flag definitions with required fields (name, description, default)
- **Documentation Integration**: Descriptions in YAML automatically become comments in generated code
- **Build System Integration**: Single definition generates multiple output formats (C macros, C++ templates, CMake variables)
- **Version Control Friendly**: Human-readable diffs make it easy to track flag changes over time
- **Validation**: The system can validate that all flags in configuration files are actually defined

### What Gets Generated

From the YAML definition file, the build system generates several artifacts:

#### 1. C/C++ Header File (`alkos/generated/include/autogen/feature_flags.h`)

**C Macros**: Each feature flag becomes a preprocessor macro:
```c
// run_test_mode - Instead of proceeding to usual kernel boot enter the test mode
#define FEATURE_FLAG_RUN_TEST_MODE 1

// debug_spinlock - Enables additional debug information for spinlocks
#define FEATURE_FLAG_DEBUG_SPINLOCK 0
```

**C++ Template Specializations**: Type-safe compile-time feature checking:
```cpp
enum class FeatureFlag {
    kRunTestMode,
    kDebugSpinlock,
    kDebugOutput,
    kDebugTraces,
    kLast,
};

template <FeatureFlag Flag>
constexpr bool FeatureEnabled = false;

// Specialized templates for each flag
template <> constexpr bool FeatureEnabled<FeatureFlag::kRunTestMode> = true;
template <> constexpr bool FeatureEnabled<FeatureFlag::kDebugSpinlock> = false;
```

#### 2. CMake Configuration (`config/conf.generated.cmake`)

CMake variables for build system integration:
```cmake
set(CMAKE_FEATURE_FLAG_RUN_TEST_MODE ON)
set(CMAKE_FEATURE_FLAG_DEBUG_SPINLOCK OFF)
set(CMAKE_FEATURE_FLAG_DEBUG_OUTPUT OFF)
set(CMAKE_FEATURE_FLAG_DEBUG_TRACES OFF)
```

#### 3. Runtime Configuration (`config/feature_flags.conf`)

Bash associative array for build script usage:
```bash
declare -A CONFIGURE_FEATURE_FLAGS

# run_test_mode - Instead of proceeding to usual kernel boot enter the test mode
CONFIGURE_FEATURE_FLAGS["run_test_mode"]=false

# debug_spinlock - Enables additional debug information for spinlocks  
CONFIGURE_FEATURE_FLAGS["debug_spinlock"]=false
```

### How to Add a New Feature Flag

#### Step 1: Define the Flag in YAML

Add your new flag to `config/feature_flags_defs.yaml`:

```yaml
feature_flags:
  # ... existing flags ...
  - name: my_new_feature
    description: Enable experimental new feature that does something amazing
    default: false
```

**Naming Convention**: Use snake_case for flag names. The system will automatically convert to appropriate naming conventions for each target language.

#### Step 2: Run the Configure Script

The next time you run `./scripts/configure/configure.bash`, the system will:
1. Detect the new flag definition
2. Add it to the user's `feature_flags.conf` with the default value
3. Generate updated C/C++ headers and CMake variables

#### Step 3: Use the Flag in Code

**In C code:**
```c
#include "autogen/feature_flags.h"

void some_function() {
#if FEATURE_FLAG_MY_NEW_FEATURE
    // Code that runs when feature is enabled
    experimental_new_functionality();
#else
    // Fallback code
    standard_functionality();
#endif
}
```

**In C++ code:**
```cpp
#include "autogen/feature_flags.h"

void some_function() {
    if constexpr (FeatureEnabled<FeatureFlag::kMyNewFeature>) {
        // Code that runs when feature is enabled
        experimental_new_functionality();
    } else {
        // Fallback code - this branch is completely eliminated at compile time
        standard_functionality();
    }
}
```

**In CMake files:**
```cmake
if(CMAKE_FEATURE_FLAG_MY_NEW_FEATURE)
    target_sources(my_target PRIVATE experimental_source.cpp)
else()
    target_sources(my_target PRIVATE standard_source.cpp)
endif()
```
