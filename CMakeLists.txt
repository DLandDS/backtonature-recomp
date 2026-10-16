cmake_minimum_required(VERSION 3.20)
project(BackToNatureDecompiled NONE)

# GCC 2.95/PSY-Q is driven by the verified Make build, rather than pretending
# that a modern host C compiler can produce the retail executable.
find_program(BTN_MAKE NAMES gmake make REQUIRED)
set(BTN_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

add_custom_target(game ALL
    COMMAND "${BTN_MAKE}" --no-print-directory -C "${BTN_ROOT}" verify
    USES_TERMINAL)
foreach(target IN ITEMS setup split verify match test check)
    add_custom_target("${target}"
        COMMAND "${BTN_MAKE}" --no-print-directory -C "${BTN_ROOT}" "${target}"
        USES_TERMINAL)
endforeach()

file(GLOB_RECURSE BTN_SOURCES CONFIGURE_DEPENDS src/*.c include/*.h)
add_custom_target(source_files SOURCES ${BTN_SOURCES})
