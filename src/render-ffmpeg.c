#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "core/render.h"
#include "raylib/raylib.h"

struct CanimRender {
    RenderTexture buffer;

    pid_t pid;
    int pipe;
    unsigned width, height, fps;
};

static inline bool initializeFFMPEG(int pipefd[2], pid_t forked[], unsigned width, unsigned height, unsigned fps)
{
    if(pipe(pipefd) < 0) {
        fprintf(stderr, "[ERROR] Couldn't create a pipe: %s\n", strerror(errno));
        return 0;
    }

    *forked = fork();
    if(*forked < 0) {
        fprintf(stderr, "[ERROR] Couldn't fork a child: %s\n", strerror(errno));
        return 0;
    }

    if (*forked == 0) {
        if (dup2(pipefd[0], STDIN_FILENO) < 0) {
            fprintf(stderr, "[ERROR]: Couldn't reopen read end of pipe as stdin: %s\n", strerror(errno));
            exit(1);
        }

        if (dup2(pipefd[0], STDOUT_FILENO) < 0) {
            fprintf(stderr, "[ERROR]: Couldn't reopen read end of pipe as stdout: %s\n", strerror(errno));
            exit(1);
        }

        close(pipefd[1]);

        char resolution[64];
        snprintf(resolution, sizeof(resolution), "%ux%u", width, height);
        char framerate[64];
        snprintf(framerate, sizeof(framerate), "%u", fps);

        int ret = execlp("ffmpeg",
            "ffmpeg",
            "-hide_banner",
            "-loglevel", "error",
            "-y",

            "-f", "rawvideo",
            "-pix_fmt", "rgba",
            "-s", resolution,
            "-r", framerate,
            "-i", "-",

            "-c:v", "libx264",
            "-vb", "2500k",
            "-c:a", "aac",
            "-ab", "200k",
            "-pix_fmt", "yuv420p",
            "output.mp4",

            NULL
        );
        if (ret < 0) {
            fprintf(stderr, "[ERROR]: Couldn't run ffmpeg as a child process: %s\n", strerror(errno));
            exit(1);
        }

        assert(0 && "unreachable");
    }

    close(pipefd[0]);
    return true;
}

CanimRender* canimRenderStart(unsigned width, unsigned height, unsigned fps, bool output)
{
    CanimRender* renderer = malloc(sizeof *renderer);
    if(!renderer) {
        fprintf(stderr, "[ERROR] Couldn't allocate a memory for the renderer\n");
        return 0;
    }

    int pipefd[2];
    pid_t forked = -1;

    if(output)
        if(!initializeFFMPEG(pipefd, &forked, width, height, fps)) {
            free(renderer);
            return 0;
        }

    renderer->pid = forked;
    renderer->pipe = pipefd[1];
    renderer->width = width;
    renderer->height = height;
    renderer->fps = fps;

    renderer->buffer = LoadRenderTexture(width, height);

    return renderer;
}

bool canimIsRendering(CanimRender* ctx) { return ctx->pid > -1; }
unsigned canimGetFramerate(CanimRender* ctx) { return ctx->fps; }

void canimRenderSetRendering(CanimRender* renderer, bool rendering)
{
    if(!renderer || rendering == canimIsRendering(renderer)) return;

    int pipefd[2];
    pid_t forked = -1;

    if(rendering && initializeFFMPEG(pipefd, &forked, renderer->width, renderer->height, renderer->fps)) {
        renderer->pid = forked;
        renderer->pipe = pipefd[1];
    }
}

void canimRenderFinish(CanimRender* renderer)
{
    if(!renderer) return;

    if(canimIsRendering(renderer)) {
        close(renderer->pipe);
        waitpid(renderer->pid, NULL, 0);
    }

    UnloadRenderTexture(renderer->buffer);
    free(renderer);
}

static inline void sendFrame(CanimRender* renderer, uint32_t* data)
{
    write(renderer->pipe, data, sizeof(*data) * renderer->width * renderer->height);
}

static inline void sendFlippedFrame(CanimRender* renderer, uint32_t* data)
{
    for (size_t y = renderer->height; y > 0; --y)
        write(renderer->pipe, &data[(y - 1) * renderer->width], sizeof(*data) * renderer->width);
}

void canimRenderSendFrame(CanimRender* renderer)
{
    if(!renderer || !canimIsRendering(renderer)) return;
    Image image = LoadImageFromTexture(renderer->buffer.texture);
    sendFlippedFrame(renderer, (uint32_t*)image.data);
    UnloadImage(image);
}

void canimRenderBeginOutput(CanimRender* renderer)
{
    if(!renderer) return;
    BeginTextureMode(renderer->buffer);
}

void canimRenderEndOutput(CanimRender* renderer)
{
    if(!renderer) return;
    EndTextureMode();
}

void canimRenderDraw(CanimRender *renderer, int x, int y, int width, int height) {
    if(!renderer) return;
    DrawTexturePro(renderer->buffer.texture,
                  (Rectangle){0, 0, (float)renderer->width, -(float)renderer->height},
                  (Rectangle){x, y, width, height},
                  (Vector2){0}, 0, WHITE);
}
