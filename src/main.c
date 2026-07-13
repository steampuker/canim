#include "./core/so_loader.h"
#include "./core/render.h"
#include "./core/timeline.h"

#include "config.h"
#define ALTARR_IMPLEMENTATION
#include "altarr.h"

#include "raylib/raylib.h"

static union { double inverse_fps; double offset; } render_timing = {0.0};

static inline double getPreviewTime() {
    return GetTime() - render_timing.offset;
}
static double getRenderTime() {
    static size_t i = 0;
    i += 1;
    return i * render_timing.inverse_fps;
}

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
    if(args->render) {
        render_timing.inverse_fps = 1.0 / (double)fps;
        SetTargetFPS(0);
    }
    else {
        render_timing.offset = GetTime();
        SetTargetFPS(fps);
    }

    canimRenderInit();

    printf("Parsed args: %s, %d\n", args->path, args->render);
    unsigned samples = 8, depth_samples = 8;
    *renderer = canimRenderStart(width, height, fps, samples, args->render);
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
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
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
        canimRenderBeginOutput(rend);
        ClearBackground(BLACK);

        if(!canimTimelineIterate(timeline, parsed_args.render ? getRenderTime() : getPreviewTime()))
            break;

        canimRenderEndOutput(rend);

        BeginDrawing();
        ClearBackground(BLACK);

        if(!parsed_args.render) {
            canimRenderDraw(rend, 0, 0, GetScreenWidth(), GetScreenHeight());
            DrawText(TextFormat("Total Time: %f, started with %f", getPreviewTime(), GetTime()), 0, GetScreenHeight() - 24, 24, (Color){255, 255, 255, 128});
            DrawFPS(0, 0);
        } else {
            const Vector2 text_size = {(GetScreenWidth() - MeasureText("Rendering", 48)) / 2., GetScreenHeight() / 2.};
            BeginDrawing();
            ClearBackground(BLACK);
            DrawText("Rendering...", text_size.x, text_size.y, 48, WHITE);
        }

        EndDrawing();

        canimRenderSendFrame(rend);
    }

    deinit(timeline, rend);
}
