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
#define APP_COMPONENTS_CORE_IMPL
#include "app-components-core.h"

/* Boring but important memory management hooks */
/*
ECS_DTOR(AppDmonEvent, ptr, {
    ecs_os_free(ptr->filepath);
    ecs_os_free(ptr->oldfilepath);
    ecs_os_free(ptr->rootdir);
})

ECS_MOVE(AppDmonEvent, dst, src, {
    ecs_os_free(dst->filepath);
    ecs_os_free(dst->oldfilepath);
    ecs_os_free(dst->rootdir);
    SDL_copyp(dst, src);
    SDL_zerop(src); // mandatory to not have double-free, dtor still called after move
})

ECS_COPY(AppDmonEvent, dst, src, {
    ecs_os_free(dst->filepath);
    ecs_os_free(dst->oldfilepath);
    ecs_os_free(dst->rootdir);
    dst->watch_id = src->watch_id;
    dst->action = src->action;
    dst->rootdir = ecs_os_strdup(src->rootdir);
    dst->filepath = ecs_os_strdup(src->filepath);
    dst->oldfilepath = ecs_os_strdup(src->oldfilepath);
    dst->user = src->user;
})
*/
APP_API void AppComponentsCoreImport(ecs_world_t *world) {
    // https://www.flecs.dev/flecs/md_docs_2EntitiesComponents.html#registration
    // See the "modules" example
    ECS_MODULE(world, AppComponentsCore);

    /* Phases for pipelines */
    RenderingPreImGui = ecs_entity_init(world, &(ecs_entity_desc_t){
            .name = "RenderingPreImGui",
            .add = ecs_ids(EcsPhase)
            });
    ecs_add_pair(world, RenderingPreImGui, EcsDependsOn, EcsPostUpdate);

    RenderingOnImGui = ecs_entity_init(world, &(ecs_entity_desc_t){
            .name = "RenderingOnImGui",
            .add = ecs_ids(EcsPhase)
            });
    ecs_add_pair(world, RenderingOnImGui, EcsDependsOn, RenderingPreImGui);

    RenderingPostImGui = ecs_entity_init(world, &(ecs_entity_desc_t){
            .name = "RenderingPostImGui",
            .add = ecs_ids(EcsPhase)
            });
    ecs_add_pair(world, RenderingPostImGui, EcsDependsOn, RenderingOnImGui);

    /* Elements for states represented as exclusive relationships */
    ECS_TAG_DEFINE(world, AppState);
    ecs_add_id(world, AppState, EcsExclusive);
    ECS_ENTITY_DEFINE(world, AppStateIdling);
    ECS_ENTITY_DEFINE(world, AppStateStreamingOrRecording);

    ECS_TAG_DEFINE(world, AppQuitState);
    ecs_add_id(world, AppQuitState, EcsExclusive);
    ECS_ENTITY_DEFINE(world, AppQuitStateResquested);
    ECS_ENTITY_DEFINE(world, AppQuitStateWaitingUserReply);
    ECS_ENTITY_DEFINE(world, AppQuitStateAccepted);

    ECS_TAG_DEFINE(world, IOState);
    ecs_add_id(world, IOState, EcsExclusive);
    ECS_ENTITY_DEFINE(world, IOComplete);
    ECS_ENTITY_DEFINE(world, IOFailure);
    ECS_ENTITY_DEFINE(world, IOCanceled);

    /* Component definitions, with metadata for REST API and/or persistence */

    // Singletons components (excluded from ecs_world_to_json() by FLECS design as in 4.1.4)
    ECS_META_COMPONENT(world, AppVersion);
    ECS_META_COMPONENT(world, AppMainTimingContext);

    ECS_COMPONENT_DEFINE(world, AppMemoryFuncs);
    ecs_struct(world, {
        .entity = ecs_id(AppMemoryFuncs),
        .members = {
            { .name = "sdl_malloc_func",  .type = ecs_id(ecs_uptr_t) },
            { .name = "sdl_calloc_func",  .type = ecs_id(ecs_uptr_t) },
            { .name = "sdl_realloc_func", .type = ecs_id(ecs_uptr_t) },
            { .name = "sdl_free_func",    .type = ecs_id(ecs_uptr_t) },
            /*
            { .name = "imgui_malloc_func",.type = ecs_id(ecs_uptr_t) },
            { .name = "imgui_free_func",  .type = ecs_id(ecs_uptr_t) },
            { .name = "imgui_allocator_functions_user_data", .type = ecs_id(ecs_uptr_t) },
            */
        }
    });

    ECS_COMPONENT_DEFINE(world, AppSDLContext);
    ecs_struct(world, {
        .entity = ecs_id(AppSDLContext),
        .members = {
            { .name = "main_window",  .type = ecs_id(ecs_uptr_t) },
            { .name = "gpu_device",   .type = ecs_id(ecs_uptr_t) },
            { .name = "sdl_io_queue", .type = ecs_id(ecs_uptr_t) },
        }
    });

    /*
    ECS_COMPONENT_DEFINE(world, AppImGuiContext);
    ecs_struct(world, {
        .entity = ecs_id(AppImGuiContext),
        .members = {
            { .name = "imgui_context", .type = ecs_id(ecs_uptr_t) },
            { .name = "imgui_io",      .type = ecs_id(ecs_uptr_t) },
        }
    });
*/
    /* Regular components */
    ECS_META_COMPONENT(world, AppActionRequested);
/*
    ECS_META_COMPONENT(world, AppHotkeyBinding);
    */

    ECS_COMPONENT_DEFINE(world, AsyncIOOutcome);
    ecs_struct(world, {
        .entity = ecs_id(AsyncIOOutcome),
        .members = {
            { .name = "asyncio", .type = ecs_id(ecs_uptr_t) },
            { .name = "type", .type = ecs_id(ecs_i32_t) },
            { .name = "result", .type = ecs_id(ecs_i32_t) },
            { .name = "buffer", .type = ecs_id(ecs_uptr_t) },
            { .name = "offset", .type = ecs_id(ecs_u64_t) },
            { .name = "bytes_requested", .type = ecs_id(ecs_u64_t) },
            { .name = "bytes_transferred", .type = ecs_id(ecs_u64_t) },
            { .name = "userdata", .type = ecs_id(ecs_uptr_t) },
        }
    });

    /*
    ECS_META_COMPONENT(world, AppDmonEvent);
    */

    /* Boring but important memory management hooks */
    /*
    ecs_set_hooks(world, AppDmonEvent, {
        //.ctor = ecs_ctor(AppDmonEvent),
        .move = ecs_move(AppDmonEvent),
        .copy = ecs_copy(AppDmonEvent),
        .dtor = ecs_dtor(AppDmonEvent),
    });
    */
}
