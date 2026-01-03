message(STATUS "    -> gui_test2")
alkos_find_sources(GUI_TEST2_SOURCES)
alkos_register_userspace_app(gui_test2 "${GUI_TEST2_SOURCES}")

add_library(gui_test2.libgraphics STATIC ${LIBGRAPHICS_SOURCES})
target_link_libraries(gui_test2.libgraphics PUBLIC
    alkos.libgraphics.interface
    alkos.libc.interface
)

target_compile_options(gui_test2.libgraphics PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:
        -fno-builtin
        -fno-rtti
        -fno-exceptions
        -fno-stack-protector
        -mcmodel=small
        -mno-red-zone
    >
)

target_link_libraries(gui_test2 PRIVATE
    gui_test2.libgraphics
)
