#include "./loader.h"
#include "./timeline.h"

#include "config.h"
#define PARG_IMPLEMENTATION
#include "parg.h"

#include "canim.h"

#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <string.h>

static struct  {
    CanimTimeline *timeline;
    void* handle;

    int width, height, fps;
} loader_state = {
    .width = RENDER_DEFAULT_WIDTH,
    .height = RENDER_DEFAULT_HEIGHT,
    .fps = RENDER_DEFAULT_FPS
};

static inline void* openLibrary(const char* path)
{
    if(path[0] == '/' || path[0] == '.')
        return dlopen(path, RTLD_LAZY | RTLD_GLOBAL);

    char current_path[1024];
    if(getcwd(current_path, 1024) != current_path) return false;

    unsigned path_len = strlen(current_path);
    current_path[path_len] = '/';
    current_path[path_len + 1] = '\0';

    return dlopen(strcat(current_path, path), RTLD_LAZY | RTLD_LOCAL);
}

bool canimLoad(const char* path, CanimTimeline *timeline, unsigned *width, unsigned *height, unsigned *fps)
{
    if(!timeline)
        return false;

    loader_state.timeline = timeline;
    canimTimelineSetLength(timeline, 0);

    if(!(loader_state.handle = openLibrary(path)))
        return fprintf(stderr, "Failed to load library: %s\n", dlerror()), false;

    void(*setup)(void) = (void(*)(void))dlsym(loader_state.handle, "canimSetup");
    setup();

    if(canimTimelineGetLength(timeline) == 0.0)
        canimTimelineComputeLength(timeline);

    *width  = loader_state.width;
    *height = loader_state.height;
    *fps    = loader_state.fps;

    return true;
}

void canimUnload(void)
{
    dlclose(loader_state.handle);
}

void canimLoaderParseArgs(int argc, char** argv, CanimArgs *result)
{
    if(!result) return;

    result->render = false;

    int longindex, c;
    struct parg_state parg = {0};
    const struct parg_option opts[] = {
        {"render", PARG_NOARG, 0, 'r'},

    };

    parg_init(&parg);

    printf("Proccesing args...\n");
    while((c = parg_getopt_long(&parg, argc, argv, "r", opts, &longindex)) >= 0)
        switch(c) {
            case 1: printf("Proccesed non-opt\n"); result->path = parg.optarg; break;
            case 'r': printf("Proccesed render\n"); result->render = true; break;
        }

}

/* Library API implementation */
unsigned canimGetRenderWidth(void)  { return loader_state.width; }
unsigned canimGetRenderHeight(void) { return loader_state.height; }
unsigned canimGetRenderFPS(void)    { return loader_state.fps; }

void canimSetRenderWidth(double width)   { loader_state.width = width; }
void canimSetRenderHeight(double height) { loader_state.height = height; }
void canimSetRenderFPS(double fps)       { loader_state.fps = fps; }

void canimAddAnimation(double start, double end, void(*callback)(double, double)) { canimTimelineAddEntry(loader_state.timeline, start, end, callback, 0, 0); }
void canimAddAnimationManaged(double start, double end, void (*callback)(double progress, double actual), void (*init)(void), void (*deinit)(void)) { canimTimelineAddEntry(loader_state.timeline, start, end, callback, init, deinit); }
void canimSetTotalLength(double seconds) { canimTimelineSetLength(loader_state.timeline, seconds); }
double canimGetTotalLength(void) { return canimTimelineGetLength(loader_state.timeline); }
