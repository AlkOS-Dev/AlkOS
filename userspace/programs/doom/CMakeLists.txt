message(STATUS "    -> doom")

# Doom source files for AlkOS
set(DOOM_SOURCES
    src/dummy.c
    src/am_map.c
    src/doomdef.c
    src/doomstat.c
    src/dstrings.c
    src/d_event.c
    src/d_items.c
    src/d_iwad.c
    src/d_loop.c
    src/d_main.c
    src/d_mode.c
    src/d_net.c
    src/f_finale.c
    src/f_wipe.c
    src/g_game.c
    src/hu_lib.c
    src/hu_stuff.c
    src/info.c
    src/i_cdmus.c
    src/i_endoom.c
    src/i_joystick.c
    src/i_scale.c
    src/i_sound.c
    src/i_system.c
    src/i_timer.c
    src/memio.c
    src/m_argv.c
    src/m_bbox.c
    src/m_cheat.c
    src/m_config.c
    src/m_controls.c
    src/m_fixed.c
    src/m_menu.c
    src/m_misc.c
    src/m_random.c
    src/mus2mid.c
    src/p_ceilng.c
    src/p_doors.c
    src/p_enemy.c
    src/p_floor.c
    src/p_inter.c
    src/p_lights.c
    src/p_map.c
    src/p_maputl.c
    src/p_mobj.c
    src/p_plats.c
    src/p_pspr.c
    src/p_saveg.c
    src/p_setup.c
    src/p_sight.c
    src/p_spec.c
    src/p_switch.c
    src/p_telept.c
    src/p_tick.c
    src/p_user.c
    src/r_bsp.c
    src/r_data.c
    src/r_draw.c
    src/r_main.c
    src/r_plane.c
    src/r_segs.c
    src/r_sky.c
    src/r_things.c
    src/sha1.c
    src/sounds.c
    src/statdump.c
    src/st_lib.c
    src/st_stuff.c
    src/s_sound.c
    src/tables.c
    src/v_video.c
    src/wi_stuff.c
    src/w_checksum.c
    src/w_file.c
    src/w_file_stdc.c
    src/w_main.c
    src/w_wad.c
    src/z_zone.c
    src/i_input.c
    src/i_video.c
    src/doomgeneric.c
    src/doomgeneric_alkos.c
)

set_source_files_properties(
        ${DOOM_SOURCES}
        PROPERTIES
        LANGUAGE C
)

alkos_register_userspace_app(doom "${DOOM_SOURCES}")

# Set C standard to C99
set_property(TARGET doom PROPERTY C_STANDARD 99)
set_property(TARGET doom PROPERTY C_STANDARD_REQUIRED OFF)

############################ Generate config.h ############################

set(HAVE_STDBOOL_H  1)
set(HAVE_STDINT_H   1)
set(HAVE_STDLIB_H   1)
configure_file(src/config.h.in src/config.h)

target_include_directories(doom PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/src
)

add_library(doom.libgraphics STATIC ${LIBGRAPHICS_SOURCES})
target_link_libraries(doom.libgraphics PUBLIC
        alkos.libgraphics.interface
        alkos.libc.interface
)

target_compile_options(doom.libgraphics PRIVATE
    $<$<COMPILE_LANGUAGE:CXX>:
        -fno-builtin
        -fno-rtti
        -fno-exceptions
        -fno-stack-protector
        -mcmodel=small
        -mno-red-zone
    >
)

target_link_libraries(doom PRIVATE
        doom.libgraphics
)
