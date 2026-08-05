#include "app.h"

SDL_AppResult SDL_AppIterate(void *appstate) {
    ecs_world_t *world = (ecs_world_t *)appstate;
    ecs_progress(world, 0.0f);
    SDL_Delay(16);
    return SDL_APP_CONTINUE;
}
