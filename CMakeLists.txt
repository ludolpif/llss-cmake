cmake_minimum_required(VERSION 3.31)
project(llss VERSION 0.1.0)
# Targeted platforms: Linux,MacOS(arm64;x86_64),Windows

include(GNUInstallDirs)
#include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/DynamicLibHelpers.cmake")
#
#option(SKIP_SDL3_COPY_DYNAMIC_LIB "Skip copying the SDL3 dynamic library to the build binary directory" OFF)
#option(SKIP_SDL3_INSTALL_DYNAMIC_LIB "Skip install the SDL3 dynamic library" OFF)
#option(SKIP_RPATH_FOR_VENDORED_DYNAMIC_LIBS "Skip RPATH tweaking for vendored dynamic library" OFF)

# set the output directory for built objects.
# This makes sure that the SDL dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# FIXME prevent installing to system directories
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}" CACHE INTERNAL "./install")

# on Web targets, we need CMake to generate a HTML webpage
if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html" CACHE INTERNAL "")
endif()

if ((APPLE AND NOT CMAKE_SYSTEM_NAME MATCHES "Darwin") OR EMSCRIPTEN)
    set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")    # Disable shared builds on platforms where it does not make sense to use them
    set(SDL_SHARED OFF)
else()
    set(SDL_SHARED ON)
endif()

if(MSVC)
    # parallelize each target, unless Ninja is the generator
    if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
        add_definitions(/MP)
    endif()
endif()

# To get an app icon on Apple platforms, add it to your executable. Afterward, the image file in Info.plist.in.
if(LINUX)
    find_package(Threads REQUIRED)
endif()

# Configure SDL by calling its CMake file.
# we use EXCLUDE_FROM_ALL so that its install targets and configs don't
# pollute upwards into our configuration.
add_subdirectory(vendored/platform/SDL EXCLUDE_FROM_ALL)

# If you don't want SDL_ttf, then remove this section.
set(SDLTTF_VENDORED ON) # tell SDL_ttf to build its own dependencies
add_subdirectory(vendored/platform/SDL_ttf EXCLUDE_FROM_ALL)

# SDL_mixer (used for playing audio)
set(SDLMIXER_MIDI_NATIVE OFF)
set(SDLMIXER_GME OFF)
set(SDLMIXER_WAVPACK OFF)
set(SDLMIXER_MOD OFF)
set(SDLMIXER_MOD_XMP OFF)
set(SDLMIXER_OPUS OFF)
set(SDLMIXER_MP3_MPG123 OFF)
set(SDLMIXER_VORBIS_VORBISFILE OFF)
set(SDLMIXER_FLAC_LIBFLAC OFF)
set(SDLMIXER_VENDORED ON) # tell SDL_mixer to build its own dependencies
add_subdirectory(vendored/platform/SDL_mixer EXCLUDE_FROM_ALL)

# SDL_image (used for loading various image formats)
set(SDLIMAGE_VENDORED ON)
set(SDLIMAGE_AVIF OFF)
set(SDLIMAGE_BMP OFF)
set(SDLIMAGE_JPG OFF)
set(SDLIMAGE_WEBP OFF)
set(SDLIMAGE_PNG_LIBPNG OFF) # SDL3 includes built-in support for loading PNGs. If you need more than what that supports, enable this option.
add_subdirectory(vendored/platform/SDL_image EXCLUDE_FROM_ALL)

# Non-SDL stuff
add_subdirectory(vendored/ui EXCLUDE_FROM_ALL)

add_executable(llss
    program/app-components-core.c
    program/app-systems-core.c
    program/main.c
    program/sdl-app-init.c
    program/sdl-app-iterate.c
    program/sdl-app-event.c
    vendored/ecs/flecs/flecs.c
)
if(APPLE)
    target_sources(llss PRIVATE "assets/logo.png")
endif()

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

target_link_libraries(llss PRIVATE
    imgui
    SDL3_image::SDL3_image
    SDL3_mixer::SDL3_mixer
    SDL3_ttf::SDL3_ttf
    SDL3::SDL3 # If using satelite libraries, SDL must be the last item in the list
)

# from https://github.com/SanderMertens/flecs/blob/master/docs/BuildingFlecs.md
if(APPLE)
    # none
elseif(LINUX)
    target_link_libraries(llss PRIVATE m rt Threads::Threads)
elseif(WIN32)
    target_link_libraries(llss PRIVATE wsock32 ws2_32 dbghelp)
endif()

# from https://cmake.org/cmake/help/latest/prop_dir/VS_STARTUP_PROJECT.html
if(WIN32)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT llss)
endif()

# Dealing with assets
# We have some non-code resources that our application needs in order to work. How we deal with those differs per platform.
if (APPLE)
    # on Apple targets, the application bundle has a "resources" subfolder where we can place our assets.
    # SDL_GetBasePath() gives us easy access to that location.
    set(input_root "${CMAKE_CURRENT_LIST_DIR}/assets")
    macro(add_resource FILE)
        file(RELATIVE_PATH relpath "${input_root}" "${FILE}")
        get_filename_component(relpath "${relpath}" DIRECTORY)
        target_sources(${EXECUTABLE_NAME} PRIVATE ${FILE})
        set_property(SOURCE ${FILE} PROPERTY MACOSX_PACKAGE_LOCATION "Resources/${relpath}")
    endmacro()
    add_resource("${CMAKE_CURRENT_LIST_DIR}/assets/Inter-VariableFont.ttf")
    add_resource("${CMAKE_CURRENT_LIST_DIR}/assets/the_entertainer.ogg")
    add_resource("${CMAKE_CURRENT_LIST_DIR}/assets/gs_tiger.svg")
elseif(EMSCRIPTEN)
    # on the web, we have to put the files inside of the webassembly
    # somewhat unintuitively, this is done via a linker argument.
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/assets/Inter-VariableFont.ttf@/\""
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/assets/the_entertainer.ogg@/\""
        "--preload-file \"${CMAKE_CURRENT_LIST_DIR}/assets/gs_tiger.svg@/\""
    )
else()
    macro(copy_helper filename)
        # for platforms that do not have a good packaging format, all we can do is copy the assets to the process working directory.
        add_custom_command(POST_BUILD
            TARGET llss
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_LIST_DIR}/assets/${filename}" $<TARGET_FILE_DIR:llss>
    )
    endmacro()
    copy_helper("Inter-VariableFont.ttf")
    copy_helper("the_entertainer.ogg")
    copy_helper("gs_tiger.svg")
endif()

# set some extra configs for each platform
set(RESOURCE_FILES "assets/logo.png")

set_target_properties(llss PROPERTIES
    # On macOS, make a proper .app bundle instead of a bare executable
    MACOSX_BUNDLE TRUE
    # in Xcode, create a Scheme in the schemes dropdown for the app.
    XCODE_GENERATE_SCHEME TRUE
    # Identification for Xcode
    XCODE_ATTRIBUTE_CURRENTYEAR "${CURRENTYEAR}"
    RESOURCE "${RESOURCE_FILES}"
)

# On macOS Platforms, ensure that the bundle is valid for distribution by calling fixup_bundle.
if(MACOS)
    # tell Install about the target, otherwise fixup won't know about the transitive dependencies
    install(TARGETS ${EXECUTABLE_NAME}
        BUNDLE DESTINATION ./install COMPONENT Runtime
        RUNTIME DESTINATION ./install/bin COMPONENT Runtime
    )

    set(DEP_DIR "${CMAKE_BINARY_DIR}")  # where to look for dependencies when fixing up
    INSTALL(CODE
        "include(BundleUtilities)
        fixup_bundle(\"$<TARGET_BUNDLE_DIR:${EXECUTABLE_NAME}>\" \"\" \"${DEP_DIR}\")
        "
    )
    set(CPACK_GENERATOR "DragNDrop")
    include(CPack)
endif()
