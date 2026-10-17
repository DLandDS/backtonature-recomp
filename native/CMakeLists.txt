include(FetchContent)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

# Hash the release archive, not a moving branch. An offline checkout can use
# -DFETCHCONTENT_SOURCE_DIR_SDL3=/path/to/SDL-release-3.2.10.
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(SDL3
    URL https://codeload.github.com/libsdl-org/SDL/tar.gz/refs/tags/release-3.2.10
    URL_HASH SHA256=8a6ddc3e70de897f7921da275ac71e375de89dad802aa2932649d1fea9068478)
FetchContent_MakeAvailable(SDL3)

option(BTN_SANITIZERS "Enable ASan and UBSan for native project code" OFF)
add_library(btn_runtime STATIC runtime.c)
target_include_directories(btn_runtime PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
add_executable(btn-native main.c)
target_link_libraries(btn-native PRIVATE btn_runtime SDL3::SDL3-static)

add_library(btn_archive STATIC archive.c archive_load.c)
target_link_libraries(btn_archive PUBLIC btn_runtime SDL3::SDL3-static)
add_executable(btn-archive archive_main.c)
target_link_libraries(btn-archive PRIVATE btn_archive)

add_library(btn_graphics STATIC graphics.c image_bank.c gte.c animation.c graphics_scene.c)
target_link_libraries(btn_graphics PUBLIC btn_archive)
add_executable(btn-graphics graphics_main.c)
target_link_libraries(btn-graphics PRIVATE btn_graphics)

add_library(btn_audio STATIC audio.c xa.c)
target_link_libraries(btn_audio PUBLIC btn_runtime SDL3::SDL3-static)
add_executable(btn-audio audio_main.c)
target_link_libraries(btn-audio PRIVATE btn_audio)

set(BTN_TARGETS btn_runtime btn-native btn_archive btn-archive btn_graphics btn-graphics btn_audio btn-audio)
if(BUILD_TESTING)
    add_executable(btn-runtime-test ../tests/test_native_runtime.c)
    target_link_libraries(btn-runtime-test PRIVATE btn_runtime)
    list(APPEND BTN_TARGETS btn-runtime-test)
    add_test(NAME native-core COMMAND btn-runtime-test)
    add_executable(btn-input-test ../tests/test_native_input.c)
    target_link_libraries(btn-input-test PRIVATE btn_runtime SDL3::SDL3-static)
    list(APPEND BTN_TARGETS btn-input-test)
    add_test(NAME native-input COMMAND btn-input-test "${CMAKE_CURRENT_BINARY_DIR}/test-saves")
    set_tests_properties(native-input PROPERTIES
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_RENDER_DRIVER=software;SDL_AUDIODRIVER=dummy")
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
    add_test(NAME native-sdl COMMAND "${Python3_EXECUTABLE}"
        "${PROJECT_SOURCE_DIR}/tests/check_native_cli.py" "$<TARGET_FILE:btn-native>")
    add_executable(btn-archive-test ../tests/test_native_archive.c)
    target_link_libraries(btn-archive-test PRIVATE btn_archive)
    list(APPEND BTN_TARGETS btn-archive-test)
    add_test(NAME native-archive COMMAND btn-archive-test)
    add_test(NAME native-archive-cli COMMAND "${Python3_EXECUTABLE}"
        "${PROJECT_SOURCE_DIR}/tests/check_archive_cli.py" "$<TARGET_FILE:btn-archive>")
    add_executable(btn-archive-load-test ../tests/test_native_archive_load.c)
    target_link_libraries(btn-archive-load-test PRIVATE btn_archive)
    list(APPEND BTN_TARGETS btn-archive-load-test)
    add_test(NAME native-archive-load COMMAND btn-archive-load-test)
    add_test(NAME native-data-tools COMMAND "${Python3_EXECUTABLE}" -m unittest discover
        -s "${PROJECT_SOURCE_DIR}/tests" -p test_prepare_native_data.py -q)
    add_executable(btn-graphics-test ../tests/test_native_graphics.c)
    target_link_libraries(btn-graphics-test PRIVATE btn_graphics)
    list(APPEND BTN_TARGETS btn-graphics-test)
    add_test(NAME native-graphics COMMAND btn-graphics-test)
    add_test(NAME native-graphics-cli COMMAND "${Python3_EXECUTABLE}"
        "${PROJECT_SOURCE_DIR}/tests/check_graphics_cli.py" "$<TARGET_FILE:btn-graphics>")
    add_executable(btn-audio-test ../tests/test_native_audio.c)
    target_link_libraries(btn-audio-test PRIVATE btn_audio)
    list(APPEND BTN_TARGETS btn-audio-test)
    add_test(NAME native-audio COMMAND btn-audio-test)
    add_test(NAME native-audio-cli COMMAND "${Python3_EXECUTABLE}"
        "${PROJECT_SOURCE_DIR}/tests/check_audio_cli.py" "$<TARGET_FILE:btn-audio>")
    add_test(NAME native-audio-tools COMMAND "${Python3_EXECUTABLE}" -m unittest discover
        -s "${PROJECT_SOURCE_DIR}/tests" -p test_prepare_native_audio.py -q)
    set(BTN_NATIVE_DATA_DIR "" CACHE PATH "Optional local HDT/BIN directory for independent member-read testing")
    if(BTN_NATIVE_DATA_DIR)
        if(NOT EXISTS "${BTN_NATIVE_DATA_DIR}/A_FILE.HDT" OR NOT EXISTS "${BTN_NATIVE_DATA_DIR}/A_FILE.BIN")
            message(FATAL_ERROR "BTN_NATIVE_DATA_DIR must contain A_FILE.HDT and A_FILE.BIN")
        endif()
        add_test(NAME native-archive-data COMMAND "${Python3_EXECUTABLE}"
            "${PROJECT_SOURCE_DIR}/tests/check_archive_cli.py" "$<TARGET_FILE:btn-archive>" "${BTN_NATIVE_DATA_DIR}")
        add_test(NAME native-graphics-data COMMAND "${Python3_EXECUTABLE}"
            "${PROJECT_SOURCE_DIR}/tests/check_graphics_cli.py" "$<TARGET_FILE:btn-graphics>" "${BTN_NATIVE_DATA_DIR}")
        add_test(NAME native-audio-data COMMAND "${Python3_EXECUTABLE}"
            "${PROJECT_SOURCE_DIR}/tests/check_audio_cli.py" "$<TARGET_FILE:btn-audio>" "${BTN_NATIVE_DATA_DIR}")
    endif()
endif()

foreach(target IN LISTS BTN_TARGETS)
    set_target_properties(${target} PROPERTIES C_STANDARD 11 C_STANDARD_REQUIRED YES C_EXTENSIONS NO)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    endif()
    if(BTN_SANITIZERS)
        if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang" AND NOT WIN32)
            target_compile_options(${target} PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
            target_link_options(${target} PRIVATE -fsanitize=address,undefined)
        else()
            message(FATAL_ERROR "BTN_SANITIZERS currently requires GCC/Clang on Linux")
        endif()
    endif()
endforeach()
