#include "app.h"
#include "app-systems-core.h"

APP_API SDL_LogPriority logpriority_earlyskip;

#define app_failure(...) do { \
    SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__); \
    return SDL_APP_FAILURE; \
} while(SDL_NULL_WHILE_LOOP_CONDITION)

#define APP_UI_DEFAULT_WIDTH 1280
#define APP_UI_DEFAULT_HEIGHT 640

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
#ifdef _DEBUG
    // Set default logging as SDL3 default logging, but with app=trace instead of app=info
    SDL_SetHint(SDL_HINT_LOGGING, "app=trace,assert=warn,test=verbose,*=error");
#endif

    /*
     * Early log message to help troubleshoot application init and allow human readable
     *  timestamps later conversion. Note that SDL_Ticks should be a CLOCK_MONOTONIC source,
     *  but some platforms may not provide it.
     *  SDL_Time is real time, subject to system clock adjustment
     */
    SDL_Time tick0_wallclock = 0;
    (void) SDL_GetCurrentTime(&tick0_wallclock);

    Uint64 tick = SDL_GetTicksNS();
    tick0_wallclock -= tick;
    app_warn("Starting %s %s with SDL %s logpriority:%d, tick0_wallclock:%"PRId64,
            APP_METADATA_NAME_STRING, APP_VERSION_STR, SDL_GetRevision(),
            logpriority_earlyskip, tick0_wallclock);
    app_info("This software is licensed with: " APP_LICENCE_NAME_STR);

    // Set metadata before SDL_Init because it will print it if loglevel is high enough
    // We don't check return value, we don't want to abort the app startup if this fails anyway.
    (void) SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_NAME_STRING, APP_METADATA_NAME_STRING);
    (void) SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_VERSION_STRING, APP_VERSION_STR);
    (void) SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_IDENTIFIER_STRING, APP_METADATA_IDENTIFIER_STRING);
    (void) SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_CREATOR_STRING, APP_METADATA_CREATOR_STRING);
    (void) SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_COPYRIGHT_STRING, APP_METADATA_COPYRIGHT_STRING);
    (void) SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_URL_STRING, APP_METADATA_URL_STRING);
    (void) SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_TYPE_STRING, APP_METADATA_TYPE_STRING);

    if (!SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_GAMEPAD | SDL_INIT_CAMERA))
        app_failure("SDL_Init(): %s", SDL_GetError());

    // Create SDL main_window graphics context
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
    SDL_WindowFlags window_flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    SDL_Window *main_window = SDL_CreateWindow(APP_METADATA_NAME_STRING,
            (int)(APP_UI_DEFAULT_WIDTH * main_scale),
            (int)(APP_UI_DEFAULT_HEIGHT * main_scale),
            window_flags);
    if (main_window == NULL)
        app_failure("SDL_CreateWindow(): %s", SDL_GetError());
    SDL_SetWindowPosition(main_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    // Assume this app will run on a single GPU and need to be restarted to change it
    bool gpu_device_debug_mode = false;
#ifdef _DEBUG
    gpu_device_debug_mode = true;
#endif
    SDL_GPUDevice *gpu_device = SDL_CreateGPUDevice(
            SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_METALLIB,
            gpu_device_debug_mode, NULL);
    if (gpu_device == NULL)
        app_failure("SDL_CreateGPUDevice(): %s", SDL_GetError());

    // Init other SDL features that aren't foreseen to may be multi-instantiated one day
    SDL_AsyncIOQueue *sdl_io_queue = SDL_CreateAsyncIOQueue();

    // Claim main_window for GPU Device
    if (!SDL_ClaimWindowForGPUDevice(gpu_device, main_window))
        app_failure("SDL_ClaimWindowForGPUDevice(): %s", SDL_GetError());

    /*
     * Try to apply a new SDL_GPUPresentMode, respecting this preference order:
     *  MAILBOX > IMMEDIATE > VSYNC
     *
     * VSYNC mode will always be supported. IMMEDIATE and MAILBOX modes may not.
     * IMMEDIATE: Immediately presents. Lowest latency option, but tearing may occur.
     * MAILBOX: Waits for vblank before presenting. No tearing is possible.
     * If there is a pending image to present, the pending image is replaced by the new image.
     * Similar to VSYNC, but with reduced visual latency.
     */

    SDL_GPUPresentMode present_mode = SDL_GPU_PRESENTMODE_VSYNC;
    // API doc says: The swapchain will be created with
    // SDL_GPU_SWAPCHAINCOMPOSITION_SDR and SDL_GPU_PRESENTMODE_VSYNC.

    bool can_mailbox = SDL_WindowSupportsGPUPresentMode(gpu_device, main_window, SDL_GPU_PRESENTMODE_MAILBOX);
    app_info("%016"PRIu64" SDL_WindowSupportsGPUPresentMode(..., MAILBOX): %s",
            SDL_GetTicksNS(), can_mailbox?"true":"false");

    if ( present_mode != SDL_GPU_PRESENTMODE_MAILBOX ) {
        // Switch to MAILBOX mode if avaiable
        if (can_mailbox) {
            if (!SDL_SetGPUSwapchainParameters(gpu_device, main_window,
                        SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_MAILBOX)) {
                app_warn("%016"PRIu64" SDL_SetGPUSwapchainParameters(..., SDR, MAILBOX) failed",
                        SDL_GetTicksNS());
            }
            present_mode = SDL_GPU_PRESENTMODE_MAILBOX;
        }
    }
    if ( present_mode != SDL_GPU_PRESENTMODE_MAILBOX ) {
        // Fallback to IMMEDIATE mode if avaiable
        bool can_immediate = SDL_WindowSupportsGPUPresentMode(gpu_device, main_window, SDL_GPU_PRESENTMODE_IMMEDIATE);
        app_info("%016"PRIu64" SDL_WindowSupportsGPUPresentMode(..., IMMEDIATE): %s",
                SDL_GetTicksNS(), can_immediate?"true":"false");

        if (can_immediate) {
            if (!SDL_SetGPUSwapchainParameters(gpu_device, main_window, SDL_GPU_SWAPCHAINCOMPOSITION_SDR, SDL_GPU_PRESENTMODE_IMMEDIATE)) {
                app_warn("%016"PRIu64" SDL_SetGPUSwapchainParameters(..., SDR, IMMEDIATE) failed",
                        SDL_GetTicksNS());
            }
            present_mode = SDL_GPU_PRESENTMODE_IMMEDIATE;
        }
    }
    app_info("%016"PRIu64" SDL_GPUPresentMode for main window: %s", SDL_GetTicksNS(),
            (present_mode==SDL_GPU_PRESENTMODE_MAILBOX)?"MAILBOX":(
                (present_mode==SDL_GPU_PRESENTMODE_IMMEDIATE)?"IMMEDIATE":(
                    (present_mode==SDL_GPU_PRESENTMODE_VSYNC)?"VSYNC":"UNKNOWN")));

    //TODO load from config
    Sint32 fr_num = 60;
    Sint32 fr_den = 1;
    int32_t ecs_worker_threads_count = 4;

    // FLECS init
    // Passing in the command line arguments will allow the explorer to display
    // the application name.
    ecs_world_t *world = ecs_init_w_args(argc, argv);
#ifdef NDEBUG
    // Note: valgrind memcheck can have hard time to converge if FLECS use multiple threads
    ecs_set_threads(world, ecs_worker_threads_count);
#else
    (void) ecs_worker_threads_count;
#endif
#ifdef app_FlecsStats
    ECS_IMPORT(world, FlecsStats); // Optional, enhance for https://www.flecs.dev/explorer
#endif
    ecs_log_set_level(0); // Increase verbosity level
    ecs_singleton_set(world, EcsRest, {0}); // Creates REST server on default port (27750)

    // Primitive type alias to ease use of ECS_STRUCT with some our choosen libs
    // See also built-in type names at flecs.h "Primitive type definitions" section
    // TODO make this more visible to mod writers (but if in AppComponentsCore module, it's not available to AppComponentsMods module)
    ecs_primitive_init(world, &(ecs_primitive_desc_t){
            .entity = ecs_entity(world, { .name = "SDL_Time" }) , .kind = EcsI64 });
    ecs_primitive_init(world, &(ecs_primitive_desc_t){
            .entity = ecs_entity(world, { .name = "ImGuiKeyChord" }) , .kind = EcsI32 });
    ecs_primitive_init(world, &(ecs_primitive_desc_t){
            .entity = ecs_entity(world, { .name = "ImGuiInputFlags" }) , .kind = EcsI32 });
    ecs_primitive_init(world, &(ecs_primitive_desc_t){
            .entity = ecs_entity(world, { .name = "dmon_watch_id" }) , .kind = EcsI32 });
    ecs_primitive_init(world, &(ecs_primitive_desc_t){
            .entity = ecs_entity(world, { .name = "dmon_action" }) , .kind = EcsI32 });

    ECS_IMPORT(world, AppSystemsCore); // Will call AppSystemsCoreImport function
    /*
    ECS_IMPORT(world, AppSystemsMods);
    ECS_IMPORT(world, AppComponentsComposition);
    */
    ecs_singleton_set(world, AppVersion, {
            .running_app_version = APP_VERSION_INT,
            .build_dep_version_compiled_against = 0 /*BUILD_DEP_VERSION_INT*/
            });
    ecs_doc_set_name(world, ecs_id(AppVersion), "Application Version");

    /*
    ecs_singleton_set(world, AppMemoryFuncs, {
            .sdl_malloc_func = alloc_count_malloc,
            .sdl_calloc_func = alloc_count_calloc,
            .sdl_realloc_func = alloc_count_realloc,
            .sdl_free_func = alloc_count_free,
            .imgui_malloc_func = alloc_count_malloc_userptr,
            .imgui_free_func = alloc_count_free_userptr,
            .imgui_allocator_functions_user_data = NULL,
            });
            */
    ecs_singleton_set(world, AppSDLContext, {
            .main_window = main_window,
            .gpu_device = gpu_device,
            .sdl_io_queue = sdl_io_queue,
            });
    /*
    ecs_singleton_set(world, AppImGuiContext, {
            .imgui_context = imgui_context,
            .imgui_io = imgui_io,
            });
            */
    ecs_singleton_set(world, AppMainTimingContext, {
            .app_iterate_count = 0,
            .total_skipped = 0,
            .main_framerate_num = fr_num,
            .main_framerate_den = fr_den,
            .main_frame_start_ns = 0,
            .main_frameid = 0,
            });
    // Set global app state. As AppState is tagged Exclusive, add_pair will replace the previous pair
    ecs_add_pair(world, ecs_id(AppSDLContext), AppState, AppStateIdling);

    // ECS First frame. This runs both the Startup, Update and user-defined systems
    ecs_progress(world, 0.0f);

    // Show window now we have a valid first image to display
    SDL_ShowWindow(main_window);

    // SDL main give us the opportunity to have a *userdata-like pointer for futher callbacks
    *appstate = world;
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppQuit(void **appstate) {
    ecs_world_t *world = (ecs_world_t *)appstate;

    const AppSDLContext *app_sdl_context = ecs_singleton_get(world, AppSDLContext);
    //const AppMainTimingContext *app_main_timing_context = ecs_singleton_get(world, AppMainTimingContext);
    //Uint32 app_iterate_count = app_main_timing_context->app_iterate_count;

    SDL_WaitForGPUIdle(app_sdl_context->gpu_device);
    /*
    cImGui_ImplSDL3_Shutdown();
    cImGui_ImplSDLGPU3_Shutdown();
    ImGui_DestroyContext(NULL);
*/
    SDL_ReleaseWindowFromGPUDevice(app_sdl_context->gpu_device, app_sdl_context->main_window);
    SDL_DestroyGPUDevice(app_sdl_context->gpu_device);
    SDL_DestroyWindow(app_sdl_context->main_window);
    SDL_DestroyAsyncIOQueue(app_sdl_context->sdl_io_queue);

    ecs_log_set_level(-1);
    ecs_fini(world);
    return SDL_APP_SUCCESS;
}
