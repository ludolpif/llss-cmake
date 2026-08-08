# from https://wiki.libsdl.org/SDL3/README-cmake#how-do-i-copy-a-sdl3-dynamic-library-to-another-location
function(sdl3_copy_dynamic_lib target_name)
    get_target_property(SDL3_TYPE SDL3::SDL3 TYPE)
    if(SDL3_TYPE STREQUAL "SHARED_LIBRARY")
       add_custom_command(
            TARGET ${target_name} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different $<TARGET_FILE:SDL3::SDL3> $<TARGET_FILE_DIR:${target_name}>
            COMMENT "Copy SDL3.dll next to the app in the build directory"
            VERBATIM
        )
    else()
        message(STATUS "SDL3 is statically linked to ${target_name}")
    endif()
endfunction()

function(sdl3_install_dynamic_lib target_name)
    get_target_property(SDL3_TYPE SDL3::SDL3 TYPE)
    if(SDL3_TYPE STREQUAL "SHARED_LIBRARY")
        # Include SDL3.dll next to the app when RUNNING THE INSTALLER
        install(FILES
            $<TARGET_FILE:SDL3::SDL3-shared>
            DESTINATION bin)
    else()
        message(STATUS "SDL3 is statically linked to ${target_name}")
    endif()
endfunction()
