#include "app.h"

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
    switch (event->type) {
        case SDL_EVENT_QUIT:
            app_info("%016"PRIu64" SDL_AppEvent(): SDL_EVENT_QUIT", SDL_GetTicksNS());
            return SDL_APP_SUCCESS;
    }
    return SDL_APP_CONTINUE;
}
