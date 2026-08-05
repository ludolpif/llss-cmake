#pragma once
#include "version-app.h"

#define VERSION_TO_INT(a, b, c) (a*10000+b*100+c)
#define VERSION_MAJOR_FROM_INT(a) (a/10000)
#define VERSION_MINOR_FROM_INT(a) ((a%10000)/100)
#define VERSION_MICRO_FROM_INT(a) (a%100)

#define APP_VERSION_INT VERSION_TO_INT(APP_VERSION_MAJOR,APP_VERSION_MINOR,APP_VERSION_PATCH)
//#define BUILD_DEP_VERSION_INT VERSION_TO_INT(BUILD_DEP_VERSION_MAJOR,BUILD_DEP_VERSION_MINOR,BUILD_DEP_VERSION_PATCH)

//-----------------------------------------------------------------------------
// [SECTION] Boring stuff needed for symbol visibility
//-----------------------------------------------------------------------------
#include <SDL3/SDL_platform_defines.h>
#ifdef app_EXPORTS
# if defined(SDL_PLATFORM_WINDOWS)
#  define APP_API __declspec(dllexport)
# else
#  if defined(__GNUC__) && __GNUC__ >= 4
#   define APP_API __attribute__ ((visibility("default")))
#  else
#   define APP_API
#  endif
# endif
#else
# if defined(SDL_PLATFORM_WINDOWS)
#  define APP_API __declspec(dllimport)
# else
#  define APP_API
# endif
#endif

#if defined(SDL_PLATFORM_WINDOWS)
# define MOD_API __declspec(dllexport)
#else
# if defined(__GNUC__) && __GNUC__ >= 4
#  define MOD_API __attribute__ ((visibility("default")))
# else
#  define MOD_API
# endif
#endif

//-----------------------------------------------------------------------------
// [SECTION] Logging helpers and counters definitions
//-----------------------------------------------------------------------------

// Convention: do not use app_info(), app_warn() for messages that can happen at each frame and flood the log, use app_debug() or counters
#define app_trace(...)    if (logpriority_earlyskip <= SDL_LOG_PRIORITY_TRACE) SDL_LogTrace(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
#define app_verbose(...)  if (logpriority_earlyskip <= SDL_LOG_PRIORITY_VERBOSE) SDL_LogVerbose(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
#define app_debug(...)    if (logpriority_earlyskip <= SDL_LOG_PRIORITY_DEBUG) SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
#define app_info(...)     SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
#define app_warn(...)     SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
#define app_error(...)    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)
#define app_critical(...) SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, __VA_ARGS__)

#include <inttypes.h> // PRIu64 and all
#include <SDL3/SDL.h>
#include "flecs.h"
