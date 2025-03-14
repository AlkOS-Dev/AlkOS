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
    set(SPEED_FLAGS "-O2")

    # Remove unwanted optimisations
    set(SPEED_FLAGS "${SPEED_FLAGS} -fno-strict-aliasing") # ERROR
    set(SPEED_FLAGS "${SPEED_FLAGS} -fno-omit-frame-pointer") # UNSAFE
    set(SPEED_FLAGS "${SPEED_FLAGS} -fno-tree-loop-vectorize") # ERROR
    set(SPEED_FLAGS "${SPEED_FLAGS} -fno-tree-slp-vectorize") # ERROR
    set(SPEED_FLAGS "${SPEED_FLAGS} -fno-delete-null-pointer-checks") # UNSAFE

    # Add tempting ones
    set(SPEED_FLAGS "${SPEED_FLAGS} -floop-unroll-and-jam")
    set(SPEED_FLAGS "${SPEED_FLAGS} -fgcse-after-reload")
    set(SPEED_FLAGS "${SPEED_FLAGS} -ftree-partial-pre")
    set(SPEED_FLAGS "${SPEED_FLAGS} -fipa-cp-clone") # Allows function cloning - might be unhandy in debugging
    set(SPEED_FLAGS "${SPEED_FLAGS} -fsplit-loops")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SPEED_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SPEED_FLAGS}")
else ()
    message(FATAL_ERROR "UNKNOWN BUILD TYPE: ${CMAKE_BUILD_TYPE}")
endif ()
