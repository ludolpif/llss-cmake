cmake_minimum_required(VERSION 3.31)
project(llss VERSION 0.1.0 LANGUAGES C CXX)
include(GNUInstallDirs)

find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)
find_package(SDL3_image REQUIRED CONFIG REQUIRED COMPONENTS SDL3_image-shared)
find_package(SDL3_ttf REQUIRED CONFIG REQUIRED COMPONENTS SDL3_ttf-shared)
if(LINUX)
    find_package(Threads REQUIRED)
endif()

# Must set before add_library()/add_executable()
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# Targeted platforms: APPLE(arm64;x86_64),LINUX,WIN32
add_executable(llss MACOSX_BUNDLE
    program/app-components-core.c
    program/app-systems-core.c
    program/main.c
    program/sdl-app-init.c
    program/sdl-app-iterate.c
    program/sdl-app-event.c
    lib/ecs/flecs/flecs.c
)

# This replaces target_include_directories()
target_sources(llss
    PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/lib/ecs/flecs"

    FILES
    "${PROJECT_SOURCE_DIR}/include/app.h"
    "${PROJECT_SOURCE_DIR}/include/app-components-core.h"
    "${PROJECT_SOURCE_DIR}/include/app-systems-core.h"
    "${PROJECT_SOURCE_DIR}/include/version-app.h"
    "${PROJECT_SOURCE_DIR}/lib/ecs/flecs/flecs.h"
)

# FLECS C needs -std=gnu99 on LINUX
target_compile_features(llss PRIVATE c_std_99)
target_compile_features(llss PRIVATE cxx_std_17)

# FLECS C needs standard-compliant preprocessor on MSVC
target_compile_options(llss PRIVATE
    $<$<C_COMPILER_ID:MSVC>:/Zc:preprocessor>
)

target_compile_definitions(llss PRIVATE app_FlecsStats)
target_compile_definitions(llss PRIVATE app_EXPORTS)

target_link_libraries(llss SDL3::SDL3)

# from https://github.com/SanderMertens/flecs/blob/master/docs/BuildingFlecs.md
if(APPLE)
    # none
elseif(LINUX)
    target_link_libraries(llss m rt Threads::Threads)
elseif(WIN32)
    target_link_libraries(llss wsock32 ws2_32 dbghelp)
endif()

install(TARGETS llss
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

# from https://cmake.org/cmake/help/latest/prop_dir/VS_STARTUP_PROJECT.html
# from https://wiki.libsdl.org/SDL3/README-cmake#how-do-i-copy-a-sdl3-dynamic-library-to-another-location
if(WIN32)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT llss)
    add_custom_command(
        TARGET llss POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy $<TARGET_FILE:SDL3::SDL3-shared> $<TARGET_FILE_DIR:llss>
        VERBATIM
    )
endif()
