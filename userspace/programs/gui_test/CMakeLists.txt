message(STATUS "    -> gui_test")
alkos_find_sources(GUI_TEST_SOURCES)
alkos_register_userspace_app(gui_test "${GUI_TEST_SOURCES}")

add_library(gui_test.libgraphics STATIC ${LIBGRAPHICS_SOURCES})
target_link_libraries(gui_test.libgraphics PUBLIC
    alkos.libgraphics.interface
    alkos.libc.interface
)

target_compile_options(gui_test.libgraphics PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:
        -fno-builtin
        -fno-rtti
        -fno-exceptions
        -fno-stack-protector
        -mcmodel=small
        -mno-red-zone
    >
)

target_link_libraries(gui_test PRIVATE
    gui_test.libgraphics
)
