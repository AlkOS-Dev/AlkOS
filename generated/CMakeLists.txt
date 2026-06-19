# SPDX-License-Identifier: MIT
# Copyright (c) 2025-2026 The AlkOS Authors
# See the AUTHORS file for the full list of contributors.

message(STATUS "Configuring Generated Files...")

############################### Preparing target ###############################

add_library(alkos.autogen.interface INTERFACE)

target_include_directories(alkos.autogen.interface PRIVATE INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/")

################################ Version files ################################

set(ALKOS_VERSION_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/autogen/version.hpp")
set(ALKOS_VERSION_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/version.cpp" CACHE INTERNAL "AlkOS embedded version source")
set(ALKOS_VERSION_SCRIPT "${CMAKE_SOURCE_DIR}/scripts/config/generate_version.bash")

set_source_files_properties(${ALKOS_VERSION_SOURCE} PROPERTIES GENERATED TRUE)

add_custom_target(alkos.version ALL
    COMMAND bash ${ALKOS_VERSION_SCRIPT} "${ARCH}" "${CMAKE_BUILD_TYPE}" "${ALKOS_OFFICIAL_BUILD}"
    BYPRODUCTS ${ALKOS_VERSION_HEADER} ${ALKOS_VERSION_SOURCE}
    COMMENT "Refreshing autogen/version.hpp and src/version.cpp"
    VERBATIM
)
