FetchContent_MakeAvailable(uacpi)

# Include uACPI's CMake configuration
include(${uacpi_SOURCE_DIR}/uacpi.cmake)

# Add list of override macro definitions
set(OVERRIDE_DEFINITIONS
        UACPI_OVERRIDE_LIBC=1
)

# Create the uACPI library
add_library(uacpi STATIC ${UACPI_SOURCES})
target_compile_definitions(uacpi PRIVATE ${OVERRIDE_DEFINITIONS})
target_include_directories(uacpi PUBLIC ${UACPI_INCLUDES})
target_include_directories(uacpi PRIVATE .)
target_link_libraries(uacpi PRIVATE libk)

# Propagate the uACPI library to the parent scope
set(THIRD_PARTY_LIBRARIES ${THIRD_PARTY_LIBRARIES} uacpi PARENT_SCOPE)
