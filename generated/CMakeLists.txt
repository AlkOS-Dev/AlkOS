message(STATUS "Configuring Generated Files...")

############################### Preparing target ###############################

add_library(alkos.autogen.interface INTERFACE)

target_include_directories(alkos.autogen.interface PRIVATE INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/")

################################ Version header ################################

set(ALKOS_VERSION_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/autogen/version.hpp")
set(ALKOS_VERSION_SCRIPT "${CMAKE_SOURCE_DIR}/scripts/config/generate_version_header.bash")

add_custom_target(alkos.version ALL
    COMMAND bash ${ALKOS_VERSION_SCRIPT} ${ARCH} ${CMAKE_BUILD_TYPE} ${ALKOS_VERSION_HEADER}
    BYPRODUCTS ${ALKOS_VERSION_HEADER}
    COMMENT "Refreshing autogen/version.hpp"
    VERBATIM
)
