# Host-Based Unit Tests

**Purpose:** Validates `libcontainers` logic (e.g., lock-free buffers) on the host OS using known to be working, sanitized Linux LibCxx/Threads implementation

**Note:** Currently usable only on x86_64 due to hardcoded cacheline size etc

**Mechanism:**
1.  **Mocks:** `mocks/` contains headers (e.g., `defines.hpp`, `atomic.hpp`) that alias kernel types to the host C++ Standard Library.
2.  **Include Priority:** CMake is configured to search `mocks/` *before* `libs/libcontainers/include`, effectively intercepting kernel dependencies.
3.  **Standalone:** Builds with the host compiler (g++/clang++), ignoring the kernel cross-compiler toolchain.

## Usage

**Warning:** Run this in a separate build directory. Do not mix with the main kernel build.

```bash
mkdir build-test
cd build-test
cmake ../tests/host
make
./container_tests
```

## Directory Layout

*   `mocks/` - Shim headers mapping kernel APIs (`hal::kcachelinesizebytes`, `u64`) to host equivalents.
*   `CMakeLists.txt` - Configures include paths and links `GTest` + `pthread`.
*   `*.cpp` - Unit tests.
