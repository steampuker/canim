#include "./so_loader.h"

#include "config.h"
#include "timeline.h"
#include <stdint.h>
#define PARG_IMPLEMENTATION
#include "parg.h"

#include "canim.h"

#include <stdbool.h>
#include <stdio.h>
#include <dlfcn.h>
#include <unistd.h>
#include <string.h>

#define INTERNAL static inline

static struct  {
    CanimTimeline *timeline;
    void* handle;

    int width, height, fps;
    bool used_camera2d;
} loader_state = {
    .width = RENDER_DEFAULT_WIDTH,
    .height = RENDER_DEFAULT_HEIGHT,
    .fps = RENDER_DEFAULT_FPS
};

INTERNAL CanimSetupInfo* getSetupInfo(void);
INTERNAL void onIteration(void);

INTERNAL void* openLibrary(const char* path)
{
    if(path[0] == '/' || path[0] == '.')
        return dlopen(path, RTLD_LAZY | RTLD_LOCAL);

    size_t length = strlen(path) + 1;

    char *temp_string = malloc((length + 2) * sizeof(char));
    temp_string[0] = '.';
    temp_string[1] = '/';
    memcpy(temp_string + 2, path, length);

    void *handle = dlopen(temp_string, RTLD_LAZY | RTLD_LOCAL);
    free(temp_string);

    return handle;
}

bool canimLoad(const char* path, CanimTimeline *timeline, unsigned *width, unsigned *height, unsigned *fps)
{
    if(!timeline)
        return false;

    loader_state.timeline = timeline;
    canimTimelineSetLength(timeline, 0);
    canimTimelineSetIterationCallback(timeline, onIteration);

    if(!(loader_state.handle = openLibrary(path)))
        return fprintf(stderr, "Failed to load library: %s\n", dlerror()), false;

    void(*setup)(CanimSetupInfo*) = (void(*)(CanimSetupInfo*))dlsym(loader_state.handle, "canimSetup");
    (void)(setup != canimSetup);
    setup(getSetupInfo());

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

#include "raylib/raylib.h"

struct CanimTexture { Texture t; };
#define AS_CAMERA2D(c) (Camera2D){.offset = {(c).offset.x, (c).offset.y}, .target = {(c).target.x, (c).target.y}, .zoom = (c).zoom, .rotation = (c).rotation}
#define AS_RECTANGLE(r) (Rectangle){.x = (r).x, .y = (r).y, .width = (r).w, .height = (r).h}
#define AS_COLOR(c) (Color){.r = (c).r, .g = (c).g, .b = (c).b, .a = (c).a}
#define AS_VECTOR2(v) (Vector2){.x = (v).x, .y = (v).y}
#define AS_VECTOR3(v) (Vector3){.x = (v).x, .y = (v).y, .z = (v).z}

INTERNAL void useCamera2D(const CanimCamera2D *cam) {
    loader_state.used_camera2d = true;
    BeginMode2D(AS_CAMERA2D(*cam));
}

INTERNAL CanimTexture loadTexture(const char* path) {
    CanimTexture texture = malloc(sizeof *texture);
    if(texture) texture->t = LoadTexture(path);
    return texture;
}

INTERNAL void unloadTexture(CanimTexture texture) {
    if(texture) UnloadTexture(texture->t);
    free(texture);
}

INTERNAL void drawRect(CanimRect rect, CanimVec2 position, float rotation, CanimColor color) { DrawRectanglePro(AS_RECTANGLE(rect), AS_VECTOR2(position), rotation, AS_COLOR(color)); }
INTERNAL void drawLine(CanimVec2 start, CanimVec2 end, float thickness, CanimColor fill_color) { DrawLineEx(AS_VECTOR2(start), AS_VECTOR2(end), thickness, AS_COLOR(fill_color)); }
INTERNAL void drawCircle(CanimVec2 position, float radius, CanimColor fill_color) { DrawCircleV(AS_VECTOR2(position), radius, AS_COLOR(fill_color)); }

INTERNAL void drawTexture(CanimTexture t, CanimVec2 pos, CanimColor color) { DrawTextureV(t->t, AS_VECTOR2(pos), AS_COLOR(color)); }
INTERNAL void drawTextureSlice(CanimTexture t, const CanimTextureSlice* slice, CanimVec2 pos, CanimColor color) {
    DrawTexturePro(t->t, AS_RECTANGLE(slice->source), (Rectangle){pos.x, pos.y, slice->destination.w, slice->destination.h}, (Vector2){slice->destination.x + slice->offset.x, slice->destination.y + slice->offset.y}, slice->rotation, AS_COLOR(color));
}

INTERNAL void addAnimation(CanimAnimation *animation) { canimTimelineAddEntry(loader_state.timeline, animation); }
INTERNAL void setVideoInfo(double render_width, double render_height, int fps) {
    loader_state.width = render_width;
    loader_state.height = render_height;
    loader_state.fps = fps;
}

INTERNAL void setTotalDuration(double duration) { canimTimelineSetLength(loader_state.timeline, duration); }

INTERNAL double getRenderWidth(void)   { return loader_state.width; }
INTERNAL double getRenderHeight(void)  { return loader_state.height; }
INTERNAL int    getRenderFPS(void)     { return loader_state.fps; }
INTERNAL double getTotalDuration(void) { return canimTimelineGetLength(loader_state.timeline); }

INTERNAL void onIteration(void) {
    if(loader_state.used_camera2d) {
        EndMode2D();
        loader_state.used_camera2d = false;
    }
}

INTERNAL CanimSetupInfo *getSetupInfo(void)
{
    const static CanimSetupInfo info = {
        .addAnimation = addAnimation,
        .setVideoInfo = setVideoInfo,
        .setTotalDuration = setTotalDuration,

        .getRenderWidth = getRenderWidth,
        .getRenderHeight = getRenderHeight,
        .getRenderFPS = getRenderFPS,
        .getTotalDuration = getTotalDuration,

        // Graphics
        .useCamera2D = useCamera2D,
        .loadTexture = loadTexture,
        .unloadTexture = unloadTexture,

        .drawTexture = drawTexture,
        .drawTextureSlice = drawTextureSlice,

        .drawLine = drawLine,
        .drawRect = drawRect,
        .drawCircle = drawCircle,
    };

    return (CanimSetupInfo*)&info;
}
