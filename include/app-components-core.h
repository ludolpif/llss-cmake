#pragma once
/*
 * This file is part of LLSS.
 *
 * LLSS is free software: you can redistribute it and/or modify it under the terms of the
 * Affero GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * LLSS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with LLSS.
 * If not, see <https://www.gnu.org/licenses/>. See LICENSE file at root of this git repo.
 *
 * Copyright 2025 ludolpif <ludolpif@gmail.com>
 */
#include "app.h"

// FLECS Reflection system boilerplate
#undef ECS_META_IMPL
#undef ONCE
#ifndef APP_COMPONENTS_CORE_IMPL
#define ECS_META_IMPL EXTERN // Ensure meta symbols are only defined once
#define ONCE extern
#else
#define ONCE
#endif

// Runtime module init, defined in program/app-component-core.c
APP_API void AppComponentsCoreImport(ecs_world_t *world);

/* Phases for pipelines */
APP_API ONCE ecs_entity_t RenderingPreImGui, RenderingOnImGui, RenderingPostImGui;

/* Elements for states represented as exclusive relationships */
APP_API ONCE ECS_TAG_DECLARE(AppState);
APP_API ONCE ECS_ENTITY_DECLARE(AppStateIdling);
APP_API ONCE ECS_ENTITY_DECLARE(AppStateStreamingOrRecording);

APP_API ONCE ECS_TAG_DECLARE(AppQuitState);
APP_API ONCE ECS_ENTITY_DECLARE(AppQuitStateResquested);
APP_API ONCE ECS_ENTITY_DECLARE(AppQuitStateWaitingUserReply);
APP_API ONCE ECS_ENTITY_DECLARE(AppQuitStateAccepted);

APP_API ONCE ECS_TAG_DECLARE(IOState);
APP_API ONCE ECS_ENTITY_DECLARE(IOComplete);
APP_API ONCE ECS_ENTITY_DECLARE(IOFailure);
APP_API ONCE ECS_ENTITY_DECLARE(IOCanceled);

/* Component definitions, with metadata for REST API and/or persistence */

// Singletons components (excluded from ecs_world_to_json() by FLECS design as in 4.1.4)
APP_API ECS_STRUCT(AppVersion, {
    int32_t running_app_version;
    int32_t build_dep_version_compiled_against;
});

typedef struct {
    SDL_malloc_func sdl_malloc_func;
    SDL_calloc_func sdl_calloc_func;
    SDL_realloc_func sdl_realloc_func;
    SDL_free_func sdl_free_func;
/*
    ImGuiMemAllocFunc imgui_malloc_func;
    ImGuiMemFreeFunc imgui_free_func;
    void* imgui_allocator_functions_user_data;
*/
    // flecs alloc_funcs can always be retreived with ecs_os_get_api()
} AppMemoryFuncs;
APP_API ONCE ECS_COMPONENT_DECLARE(AppMemoryFuncs);

typedef struct {
    SDL_Window *main_window;
    SDL_GPUDevice *gpu_device;
    SDL_AsyncIOQueue *sdl_io_queue;
} AppSDLContext;
APP_API ONCE ECS_COMPONENT_DECLARE(AppSDLContext);

/*
typedef struct {
    ImGuiContext* imgui_context;
    ImGuiIO *imgui_io;
} AppImGuiContext;
APP_API ONCE ECS_COMPONENT_DECLARE(AppImGuiContext);
*/

APP_API ECS_STRUCT(AppMainTimingContext, {
    uint32_t app_iterate_count;
    uint32_t total_skipped;
    int32_t main_framerate_num;  // AVRational framerate numerator
    int32_t main_framerate_den;  // AVRational framerate denominator
    uint64_t main_frame_start_ns; // In SDL_GetTicksNS() format, snapped to multiple of main_framerate
    uint64_t main_frameid; // Unique identifier for current frame, garanted monotonic until main_framerate changes
});

/* Regular components */

// Elements for custom action triggering from ImGui input handling like
// ImGui_Shortcut(ImGuiMod_Ctrl|ImGuiKey_Z, ImGuiInputFlags_RouteGlobal|ImGuiInputFlags_Repeat)
APP_API ECS_STRUCT(AppActionRequested, {
    ecs_entity_t source; // Could be an hotkey, an event from an API or a script
});
/*
APP_API ECS_STRUCT(AppHotkeyBinding, {
    ImGuiKeyChord key_chord;
    ImGuiInputFlags flags;
    ecs_entity_t action;
});
*/
// Elements for SDL async IO handling from the ECS
typedef SDL_AsyncIOOutcome AsyncIOOutcome;
APP_API ONCE ECS_COMPONENT_DECLARE(AsyncIOOutcome);


// Elements for file an directory monitor events (inotify, ReadDirectoryChangesW(), fswatch)
/*
APP_API ECS_STRUCT(AppDmonEvent, {
    dmon_watch_id watch_id;
    dmon_action action;
    char *rootdir;
    char *filepath;
    char *oldfilepath;
    void *user;
});
*/
