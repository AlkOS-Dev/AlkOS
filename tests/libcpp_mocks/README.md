# Host LibC/LibCpp Mocks

**Purpose:**
Provides CMake targets (`alkos.libcpp.interface`, `alkos.libc.interface`) that force kernel code to link against **Host** STL/LibC.

**Mechanism:**
- Exposes header search paths that shadow kernel headers.
- Enables reliable concurrency testing (ASan/TSan) by using the host's threaded runtime.
