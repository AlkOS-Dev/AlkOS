# Host-Based Unit Tests

**Goal:** Validate logic (esp. concurrency) on host OS using sanitizers (ASan/UBSan) and proven host STL.

**⚠️ SEPARATE PROJECT ⚠️**
This directory is a **standalone CMake project**.
- It uses the **Host Toolchain** (your system's GCC/Clang).
- The main kernel build uses a **Cross-Compiler**.
- **DO NOT** mix them. Run this in a clean `build-host-tests` directory.

**Mechanism:**
- **CMake:** `alkos_add_host_test_suite` creates a test executable.
- **Mocks:** Links `alkos.libcpp.interface` (mocks) instead of kernel libcpp. Redirects `<atomic>`, `<mutex>`, etc. to host STL.
- **Framework:** GoogleTest.

**How to Add:**
1. Create `tests/host/` directory in your library.
2. Add `.cpp` files with `TEST(...)`.
3. In CMake, call:
   ```cmake
   alkos_add_host_test_suite(
       TARGET_NAME "my_lib"
       DIRECTORY "tests/host"
       LIBS_UNDER_TEST alkos.my_lib
   )
   ```

**How to Run:**
```bash
mkdir build-host
cd build-host
cmake ../tests/host  # Points to THIS directory, not the root
make
ctest
```
