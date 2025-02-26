include(FetchContent)

set(THIRD_PARTY_LIBRARIES "")

#################################### uACPI ###################################

FetchContent_Declare(
        uacpi
        GIT_REPOSITORY https://github.com/UltraOS/uACPI.git
        GIT_TAG 4ab3a78006a930e2cda5a92f33fc84e1ec6b4a3d # 2.0.0
)

add_subdirectory(uacpi)

##############################################################################

# Propagate the third-party libraries to the parent scope
set(THIRD_PARTY_LIBRARIES ${THIRD_PARTY_LIBRARIES} PARENT_SCOPE)
