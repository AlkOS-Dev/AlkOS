include(FetchContent)

set(THIRD_PARTY_LIBRARIES "")

# Include the uACPI library
add_subdirectory(uacpi)

# Propagate the third-party libraries to the parent scope
set(THIRD_PARTY_LIBRARIES ${THIRD_PARTY_LIBRARIES} PARENT_SCOPE)
