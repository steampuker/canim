#ifndef CANIM_LIB_H
#define CANIM_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <stddef.h>
#include <stdbool.h>

struct CanimFrameContents {
    double progress;
    double global_progress;

    double animation_length;
    void * data;
};
typedef const struct CanimFrameContents* CanimFrame;

typedef struct CanimAnimation {
    void  (*callback)(CanimFrame); // Function to be called on every frame for the animation

    void* (*init)(void);    // Additional data initializer, nullable
    void  (*deinit)(void*); // Additional data deinitializer, nullable

    double start, duration;
} CanimAnimation;

typedef struct CanimSetupInfo CanimSetupInfo;

void canimSetup(CanimSetupInfo* info);

/* =======================
 |  Graphics Definition
======================= */

typedef union CanimVec2 {
    struct { float x, y; };
    float elements[2];
} CanimVec2;

typedef union CanimVec3 {
    struct { float x, y, z; };
    float elements[3];
} CanimVec3;

typedef union CanimRect {
    struct { float x, y, w, h; };
    struct { CanimVec2 position, size; };
    float elements[4];
} CanimRect;

typedef union CanimColor {
    struct { unsigned char r, g, b, a; };
    unsigned char elements[4];
} CanimColor;

typedef struct CanimCamera2D {
    CanimVec2 target, offset;
    float rotation, zoom;
} CanimCamera2D;

typedef struct CanimTexture* CanimTexture;
typedef struct CanimTextureSlice {
    CanimRect source, destination;
    CanimVec2 pivot_offset;
    float rotation;
} CanimTextureSlice;

struct CanimSetupInfo {
    void (*addAnimation)(CanimAnimation *animation);
    void (*setVideoInfo)(double render_width, double render_height, int fps);
    void (*setTotalDuration)(double duration_seconds);

    double (*getRenderWidth)(void);
    double (*getRenderHeight)(void);
    int    (*getRenderFPS)(void);
    double (*getTotalDuration)(void);

    void (*useCamera2D)(const CanimCamera2D*);

    CanimTexture (*loadTexture)(const char* filename);
    void (*unloadTexture)(CanimTexture);

    void (*drawTexture)(const CanimTexture, CanimVec2 position, CanimColor fill_color);
    void (*drawTextureSlice)(const CanimTexture, const CanimTextureSlice* slice, CanimVec2 position, CanimColor fill_color);

    void (*drawLine)(CanimVec2 start, CanimVec2 end, float thickness, CanimColor fill_color);
    void (*drawRect)(CanimRect rect, CanimVec2 position, CanimVec2 pivot_offset, float rotation, CanimColor fill_color);
    void (*drawCircle)(CanimVec2 position, float radius, CanimColor fill_color);
    void (*drawRectOutline)(CanimRect rect, CanimVec2 position, CanimVec2 pivot_offset, float rotation, float thickness, CanimColor fill_color);
    void (*drawCircleOutline)(CanimVec2 position, float radius, float thickness, CanimColor fill_color);
};

static const CanimColor CANIM_WHITE = {255, 255, 255, 255};
static const CanimColor CANIM_RED = {255, 0, 0, 255};
static const CanimColor CANIM_GREEN = {0, 255, 0, 255};
static const CanimColor CANIM_BLUE = {0, 0, 255, 255};
static const CanimColor CANIM_YELLOW = {255, 255, 0, 255};
static const CanimColor CANIM_YELLOW_GLOSSY = {245, 165, 0, 255}; // School bus yellow

// Some checks to make sure type-punning works
static_assert(offsetof(CanimVec2, y) == offsetof(CanimVec2, elements[1]), "Wrong alignment: CanimVec2");
static_assert(offsetof(CanimVec3, z) == offsetof(CanimVec2, elements[2]), "Wrong alignment: CanimVec3");
static_assert(offsetof(CanimColor, a) == offsetof(CanimColor, elements[3]), "Wrong alignment: CanimColor");
static_assert(offsetof(CanimRect, h) == offsetof(CanimRect, size.y) && offsetof(CanimRect, h) == offsetof(CanimRect, elements[3]), "Wrong alignment: CanimRect");

#ifdef __cplusplus
}
#endif

#endif
