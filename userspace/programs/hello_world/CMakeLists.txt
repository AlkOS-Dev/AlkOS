message(STATUS "    -> hello")
alkos_find_sources(HELLO_WORLD_SOURCES)
alkos_register_userspace_app(hello "${HELLO_WORLD_SOURCES}")
