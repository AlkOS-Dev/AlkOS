message(STATUS "  -> gui_test")
alkos_find_sources(GUI_TEST_SOURCES)
alkos_register_userspace_app(gui_test "${GUI_TEST_SOURCES}")
