#ifndef MINITWEEN_H
#define MINITWEEN_H

#include <stdbool.h>

typedef struct MiniTween {
    double start, end, duration;
    int tween_type;
} MiniTween;

static inline MiniTween MiniTweenCreate(double duration, double delay, MiniTween *parent) {
    MiniTween tween;
    tween.start    = delay + (parent ? parent->end : 0);
    tween.end      = tween.start + duration;
    tween.duration = duration;
    return tween;
}

static inline bool MiniTweenUpdate(const MiniTween *tween, double t, double *progress) {
    if(!progress) progress = &t;

    if(t > tween->start && t < tween->end) {
        *progress = (t - tween->start) / tween->duration;
        return 1;
    }

    *progress = (t >= tween->end) ? tween->duration : 0;
    return 0;
}

#endif
