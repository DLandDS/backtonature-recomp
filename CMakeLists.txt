cmake_minimum_required(VERSION 3.20)
project(BackToNatureDecompiled NONE)

option(BTN_BUILD_NATIVE "Build the SDL3 runtime (no game data or PSY-Q required)" OFF)
if(BTN_BUILD_NATIVE)
    set(BTN_MATCHING_DEFAULT OFF)
else()
    set(BTN_MATCHING_DEFAULT ON)
endif()
option(BTN_BUILD_MATCHING "Expose the original PS1 Make targets" ${BTN_MATCHING_DEFAULT})

# GCC 2.95/PSY-Q is driven by the verified Make build, rather than pretending
# that a modern host C compiler can produce the retail executable.
if(BTN_BUILD_MATCHING)
    find_program(BTN_MAKE NAMES gmake make REQUIRED)
    set(BTN_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")

    add_custom_target(game ALL
        COMMAND "${BTN_MAKE}" --no-print-directory -C "${BTN_ROOT}" verify
        USES_TERMINAL)
    foreach(target IN ITEMS setup split verify match check)
        add_custom_target("${target}"
            COMMAND "${BTN_MAKE}" --no-print-directory -C "${BTN_ROOT}" "${target}"
            USES_TERMINAL)
    endforeach()
    if(BTN_BUILD_NATIVE)
        # CTest owns 'test' in a combined build.
        add_custom_target(ps1-test COMMAND "${BTN_MAKE}" --no-print-directory
            -C "${BTN_ROOT}" test USES_TERMINAL)
    else()
        add_custom_target(test COMMAND "${BTN_MAKE}" --no-print-directory
            -C "${BTN_ROOT}" test USES_TERMINAL)
    endif()
endif()

file(GLOB_RECURSE BTN_SOURCES CONFIGURE_DEPENDS src/*.c include/*.h)
add_custom_target(source_files SOURCES ${BTN_SOURCES})

if(BTN_BUILD_NATIVE)
    enable_language(C)
    include(CTest)
    add_subdirectory(native)
endif()
