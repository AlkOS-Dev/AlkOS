set(CMAKE_C_FLAGS_RELEASE "" CACHE STRING "Release C flags" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "" CACHE STRING "Release C++ flags" FORCE)

# Architecture specific flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -ffreestanding -mstackrealign")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -ffreestanding -fno-exceptions -fno-rtti -mstackrealign")

# Debug or Release flags
if (CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g -fno-inline -fstack-protector-all")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -fno-inline -fstack-protector-all")
    set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -g -F dwarf")
elseif (CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RELEASE")
    set(RELEASE_FLAGS "-O2")

    # ERROR = should not be used in kernel code at all due to low level issues
    # UNSAFE = may work but it would need adaption in code and possibly will introduce strange behavior if not adapted.

    # Remove unwanted optimisations
    set(RELEASE_FLAGS "${RELEASE_FLAGS} -fno-strict-aliasing") # ERROR
    set(RELEASE_FLAGS "${RELEASE_FLAGS} -fno-tree-loop-vectorize") # ERROR
    set(RELEASE_FLAGS "${RELEASE_FLAGS} -fno-tree-slp-vectorize") # ERROR
    set(RELEASE_FLAGS "${RELEASE_FLAGS} -fno-omit-frame-pointer") # UNSAFE
    set(RELEASE_FLAGS "${RELEASE_FLAGS} -fno-delete-null-pointer-checks") # UNSAFE

    # Add tempting ones
    set(RELEASE_FLAGS "${RELEASE_FLAGS} -floop-unroll-and-jam")
    set(RELEASE_FLAGS "${RELEASE_FLAGS} -fgcse-after-reload")
    set(RELEASE_FLAGS "${RELEASE_FLAGS} -ftree-partial-pre")
    set(RELEASE_FLAGS "${RELEASE_FLAGS} -fipa-cp-clone")
    set(RELEASE_FLAGS "${RELEASE_FLAGS} -fsplit-loops")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${RELEASE_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${RELEASE_FLAGS}")
else ()
    message(FATAL_ERROR "UNKNOWN BUILD TYPE: ${CMAKE_BUILD_TYPE}")
endif ()
