# SPDX-License-Identifier: MIT
# Copyright (c) 2025-2026 The AlkOS Authors
# See the AUTHORS file for the full list of contributors.

message(STATUS "  -> Configuring for x86_64")

############################### Adding Sources ###############################

set(exclude_patterns ".*/(crti\\.nasm|crtn\\.nasm)$")
alkos_target_sources(alkos.libc.interface
  SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.nasm"
  EXCLUDE ${exclude_patterns}
)

target_include_directories(alkos.libc.interface INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

########################## CXX Global Constructors ###########################

add_library(alkos.libc.crti OBJECT
        cxx/crti.nasm
)
add_library(alkos.libc.crtn OBJECT
        cxx/crtn.nasm
)

set(CRTI_OBJ_FILE_PATH "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/alkos.libc.crti.dir/cxx/crti.nasm.o")
set(CRTN_OBJ_FILE_PATH "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/alkos.libc.crtn.dir/cxx/crtn.nasm.o")
set_property(TARGET alkos.libc.config PROPERTY CRTI_OBJ "${CRTI_OBJ_FILE_PATH}")
set_property(TARGET alkos.libc.config PROPERTY CRTN_OBJ "${CRTN_OBJ_FILE_PATH}")
