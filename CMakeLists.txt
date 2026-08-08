cmake_minimum_required(VERSION 3.31)
project(llss VERSION 0.1.0 LANGUAGES C CXX)
include(GNUInstallDirs)
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/DynamicLibHelpers.cmake")

option(SKIP_SDL3_COPY_DYNAMIC_LIB "Skip copying the SDL3 dynamic library to the build binary directory" OFF)
option(SKIP_SDL3_INSTALL_DYNAMIC_LIB "Skip install the SDL3 dynamic library" OFF)
option(SKIP_RPATH_FOR_VENDORED_DYNAMIC_LIBS "Skip RPATH tweaking for vendored dynamic library" OFF)

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)
find_package(SDL3_ttf REQUIRED)

if(LINUX)
    find_package(Threads REQUIRED)
endif()

add_subdirectory(vendored/ui)

# Targeted platforms: APPLE(arm64;x86_64),LINUX,WIN32
add_executable(llss MACOSX_BUNDLE
    program/app-components-core.c
    program/app-systems-core.c
    program/main.c
    program/sdl-app-init.c
    program/sdl-app-iterate.c
    program/sdl-app-event.c
    vendored/ecs/flecs/flecs.c
)

# This replaces target_include_directories()
target_sources(llss
    PUBLIC FILE_SET HEADERS
    BASE_DIRS
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_SOURCE_DIR}/vendored/events/dmon"
    "${PROJECT_SOURCE_DIR}/vendored/ecs/flecs"
    FILES
    "${PROJECT_SOURCE_DIR}/include/app.h"
    "${PROJECT_SOURCE_DIR}/include/app-components-core.h"
    "${PROJECT_SOURCE_DIR}/include/app-systems-core.h"
    "${PROJECT_SOURCE_DIR}/include/version-app.h"
    "${PROJECT_SOURCE_DIR}/vendored/events/dmon/dmon.h"
    "${PROJECT_SOURCE_DIR}/vendored/ecs/flecs/flecs.h"
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

target_link_libraries(llss PRIVATE imgui SDL3::SDL3)

# from https://github.com/SanderMertens/flecs/blob/master/docs/BuildingFlecs.md
if(APPLE)
    # none
elseif(LINUX)
    target_link_libraries(llss PRIVATE m rt Threads::Threads)
elseif(WIN32)
    target_link_libraries(llss PRIVATE wsock32 ws2_32 dbghelp)
endif()

install(TARGETS llss
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

if(NOT SKIP_SDL3_COPY_DYNAMIC_LIB)
	sdl3_copy_dynamic_lib(llss)
endif()
if(NOT SKIP_SDL3_INSTALL_DYNAMIC_LIB)
	sdl3_install_dynamic_lib(llss)
endif()
if(NOT SKIP_RPATH_FOR_VENDORED_DYNAMIC_LIBS)
	rpath_for_vendored_dynamic_libs(llss)
endif()

# from https://cmake.org/cmake/help/latest/prop_dir/VS_STARTUP_PROJECT.html
if(WIN32)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT llss)
endif()
