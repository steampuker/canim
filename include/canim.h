#ifndef CANIM_LIB_H
#define CANIM_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CanimAnimationFrame {
    double progress;
    double global_progress;

    double animation_length;
    void * data;
} CanimAnimationFrame;

typedef struct CanimAnimation {
    void  (*callback)(const CanimAnimationFrame* frame_data); // Function to be called on every frame for the animation

    void* (*init)(void);         // Additional data initializer, nullable
    void  (*deinit)(void* data); // Additional data deinitializer, nullable

    double start, duration;
} CanimAnimation;

void canimSetup(void);

void canimAddAnimation(CanimAnimation* animation);

double canimGetTotalLength(void);
unsigned canimGetRenderWidth(void);
unsigned canimGetRenderHeight(void);
unsigned canimGetRenderFPS(void);

void canimSetTotalLength(double seconds);
void canimSetRenderWidth(double);
void canimSetRenderHeight(double);
void canimSetRenderFPS(double);

#ifdef __cplusplus
}
#endif

#endif
