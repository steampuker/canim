#ifndef CANIM_LOADER_H
#define CANIM_LOADER_H

#include "timeline.h"
#include <stdbool.h>

typedef struct CanimArgs {
    const char* path;
    bool render;
} CanimArgs;

bool canimLoad(const char* path, CanimTimeline *timeline, unsigned *width, unsigned *height, unsigned *fps);
void canimUnload(void);
void canimLoaderParseArgs(int argc, char** argv, CanimArgs *args_result);

#endif
