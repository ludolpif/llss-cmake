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
#define APP_SYSTEMS_CORE_IMPL
#include "app-systems-core.h"
/*
#include "dcimgui_impl_sdl3.h"
#include "dcimgui_impl_sdlgpu3.h"
#ifdef _MSC_VER
  #pragma warning(push)
  #pragma warning(disable: 5287)
#endif
#include "dcimgui_internal.h" // Needed for unfinished DockBuilder API
#ifdef _MSC_VER
  #pragma warning(pop)
#endif
*/


void AppSystemsCoreImport(ecs_world_t *world) {
    // ECS_IMPORT Will call AppComponentsCoreImport(world) from app-components-core.c
    ECS_IMPORT(world, AppComponentsCore);

    ECS_MODULE(world, AppSystemsCore);

    // Tasks declarations, will run once per frame
    ECS_SYSTEM(world, InjectIOAsyncEvents, EcsOnLoad, 0);
    /*
    ECS_SYSTEM(world, ImGuiPrepareForNewFrame, RenderingPreImGui, 0);
    ECS_SYSTEM(world, ImGuiSetupDockSpace, RenderingOnImGui, 0);
    ECS_SYSTEM(world, ImGuiRenderAndSubmit, RenderingPostImGui, 0);
    */
}

void InjectIOAsyncEvents(ecs_iter_t *it) {
    const AppSDLContext *app_sdl_context = ecs_singleton_get(it->world, AppSDLContext);
    SDL_AsyncIOOutcome outcome;
    if (SDL_GetAsyncIOResult(app_sdl_context->sdl_io_queue, &outcome)) {
        ecs_entity_t e_ctx = (ecs_entity_t) outcome.userdata;
        ecs_entity_t e_outcome = IOFailure;
        switch ( outcome.result ) {
            case SDL_ASYNCIO_COMPLETE: /**< request was completed without error */
                //app_trace("%016"PRIu64" InjectIOAsyncEvents(): async IO for %s complete",
                //        SDL_GetTicksNS(), ctx_name);
                e_outcome = IOComplete;
                break;
            case SDL_ASYNCIO_FAILURE:  /**< request failed for some reason; check SDL_GetError()! */
                app_warn("%016"PRIu64" InjectIOAsyncEvents(): async IO for %s failed: %s",
                        SDL_GetTicksNS(), ecs_get_name(it->world, e_ctx), SDL_GetError());
                e_outcome = IOFailure;
                break;
            case SDL_ASYNCIO_CANCELED: /**< request was canceled before completing. */
                app_warn("%016"PRIu64" InjectIOAsyncEvents(): async IO for %s canceled",
                        SDL_GetTicksNS(), ecs_get_name(it->world, e_ctx));
                e_outcome = IOCanceled;
                break;
            default:
                app_warn("%016"PRIu64" InjectIOAsyncEvents(): async IO for %s unknown result: %"PRIi32,
                        SDL_GetTicksNS(), ecs_get_name(it->world, e_ctx), outcome.result);
                e_outcome = IOFailure;
                break;
        }
        // Ass AsyncIOOutcome component that is a direct mirror of SDL_AsyncIOOutcome struct
        ecs_set_ptr(it->world, e_ctx, AsyncIOOutcome, &outcome);

        // Set io state as pair for queries
        // As IOState is tagged Exclusive, add_pair will replace the previous pair
        ecs_add_pair(it->world, e_ctx, IOState, e_outcome);
    }
}
/*
void ImGuiPrepareForNewFrame(ecs_iter_t *it) {
    cImGui_ImplSDLGPU3_NewFrame();
    cImGui_ImplSDL3_NewFrame();
    ImGui_NewFrame();
}

void ImGuiSetupDockSpace(ecs_iter_t *it) {
    const ImGuiViewport* viewport = ImGui_GetMainViewport();

    // Set a bunch of special properties for an always backgrounded and full-screened window
    ImGui_SetNextWindowPos(viewport->WorkPos, 0);
    ImGui_SetNextWindowSize(viewport->WorkSize, 0);
    ImGui_SetNextWindowViewport(viewport->ID);
    ImGui_PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui_PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    ImGui_PushStyleVarImVec2(ImGuiStyleVar_WindowPadding, (ImVec2){ 0.0f, 0.0f });

    ImGui_Begin("DockspaceWithMenuBar", NULL, window_flags);
    ImGui_PopStyleVarEx(3);
    ImGuiID dockspace_id = ImGui_GetID("DockspaceWithMenuBar");
    // TODO query all ECS defined global bindings and set it here, to have them always routable

    // Create dockspace default layout settings if they were never defined
    if (ImGui_DockBuilderGetNode(dockspace_id) == NULL) {
        ImGui_DockBuilderAddNodeEx(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui_DockBuilderSetNodeSize(dockspace_id, viewport->Size);
        ImGuiID dock_id_right = 0;
        ImGuiID dock_id_main = dockspace_id;
        ImGui_DockBuilderSplitNode(dock_id_main, ImGuiDir_Right, 0.20f, &dock_id_right, &dock_id_main);
        ImGuiID dock_id_right_top = 0;
        ImGuiID dock_id_right_bottom = 0;
        ImGui_DockBuilderSplitNode(dock_id_right, ImGuiDir_Up, 0.50f, &dock_id_right_top, &dock_id_right_bottom);

        ecs_iter_t it2 = ecs_query_iter(it->world, ActivateLayoutVariantsQuery);
        while (ecs_query_next(&it2)) {
            for (int i = 0; i < it2.count; i++) {
                const char *name = ecs_get_name(it->world, it2.entities[i]);
                ImGui_DockBuilderDockWindow(name, dock_id_main);
            }
        }
        ImGui_DockBuilderDockWindow(_("Properties###Properties"), dock_id_right_top);
        ImGui_DockBuilderDockWindow(_("Layers###Layers"), dock_id_right_bottom);
        ImGui_DockBuilderFinish(dockspace_id);
    }
    // Submit dockspace
    ImGui_DockSpace(dockspace_id);

    // TODO query all ECS defined global bindings and set it here, to have them always routable

    ImGui_End(); // DockspaceWithMenuBar Window
}
void ImGuiRenderAndSubmit(ecs_iter_t *it) {
    const AppSDLContext *app_sdl_context = ecs_singleton_get(it->world, AppSDLContext);
    const AppImGuiContext *app_imgui_context = ecs_singleton_get(it->world, AppImGuiContext);

    SDL_GPUDevice *gpu_device = app_sdl_context->gpu_device;
    SDL_Window *main_window = app_sdl_context->main_window;
    SDL_FColor sdl_clear_color = { 0.4f, 0.4f, 0.4f, 1.0f };
    ImGuiIO *imgui_io = app_imgui_context->imgui_io;

    ImGui_Render();
    ImDrawData* draw_data = ImGui_GetDrawData();
    const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);

    // Acquire a GPU command buffer
    SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(gpu_device);

    // Acquire a target texture
    SDL_GPUTexture* swapchain_texture;
    SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, main_window, &swapchain_texture, NULL, NULL);

    if (swapchain_texture != NULL && !is_minimized) {
        // Mandatory ImGui_ImplSDLGPU3_PrepareDrawData() to upload the vertex/index buffer!
        cImGui_ImplSDLGPU3_PrepareDrawData(draw_data, command_buffer);

        // Setup and start a render pass
        SDL_GPUColorTargetInfo target_info = {};
        target_info.texture = swapchain_texture;
        target_info.clear_color = sdl_clear_color;
        target_info.load_op = SDL_GPU_LOADOP_CLEAR;
        target_info.store_op = SDL_GPU_STOREOP_STORE;
        target_info.mip_level = 0;
        target_info.layer_or_depth_plane = 0;
        target_info.cycle = false;
        SDL_GPURenderPass* render_pass = SDL_BeginGPURenderPass(command_buffer, &target_info, 1, NULL);

        // Render ImGui
        cImGui_ImplSDLGPU3_RenderDrawData(draw_data, command_buffer, render_pass);

        SDL_EndGPURenderPass(render_pass);
    }

    // Update and Render additional Platform Windows
    if (imgui_io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui_UpdatePlatformWindows();
        ImGui_RenderPlatformWindowsDefault();
    }

    SDL_SubmitGPUCommandBuffer(command_buffer);
}
*/
