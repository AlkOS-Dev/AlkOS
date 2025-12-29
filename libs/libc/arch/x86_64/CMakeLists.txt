message(STATUS "  -> Configuring for x86_64")

############################### Adding Sources ###############################

alkos_target_sources(alkos.libc.interface
  SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.nasm"
)

target_include_directories(alkos.libc.interface INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
