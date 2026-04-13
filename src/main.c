#include <assert.h>
#include "raylib/raylib.h"
#include "core/loader.h"
#include "core/render.h"
#include "core/timeline.h"
#include "utils/config.h"

#define ALTARR_IMPLEMENTATION
#include "utils/altarr.h"

void drawError(const char* error)
{
    SetTargetFPS(30);
    while(GetKeyPressed() == KEY_NULL && !WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawText(error,
            (GetScreenWidth() - MeasureText(error, 24)) / 2,
            GetScreenHeight() / 2,
            24, GRAY);
        EndDrawing();
    }
}

bool init(CanimArgs *args, CanimTimeline **timeline, CanimRender **renderer)
{
    unsigned width, height, fps;
    *timeline = canimTimelineCreate(PAGINATION_SECONDS);

    TraceLog(LOG_INFO, "Opening file: %s", args->path);
    if(!canimLoad(args->path, *timeline, &width, &height, &fps))
        return drawError("Animation file is invalid (press any key to quit)"), false;

    MaximizeWindow();
    *renderer = canimRenderStart(width, height, fps, args->render);
    return true;
}

void deinit(CanimTimeline* timeline, CanimRender *renderer)
{
    canimRenderFinish(renderer);
    canimTimelineDestroy(timeline);
    canimUnload();
}

int main(int argc, char** argv)
{
    //SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "Canim");

    CanimArgs parsed_args = {0};
    canimLoaderParseArgs(argc, argv, &parsed_args);
    if(!parsed_args.path)
        return drawError("Animation file not provided (press any key to quit)"), -1;

    CanimTimeline *timeline;
    CanimRender *rend;

    if(!init(&parsed_args, &timeline, &rend))
        return -1;

    while (!WindowShouldClose())
    {
        BeginDrawing();
        canimRenderBeginOutput(rend);
        ClearBackground(BLACK);

        if(!canimTimelineIterate(timeline, GetTime()))
            break;

        canimRenderEndOutput(rend);

        canimRenderDraw(rend, 0, 0, GetScreenWidth(), GetScreenHeight());
        //DrawFPS(0, 0);
        DrawText(TextFormat("Total Time: %f", GetTime()), 0, 0, 24, WHITE);
        EndDrawing();

        canimRenderSendFrame(rend);
    }

    deinit(timeline, rend);
}
