#ifndef CANIM_UTILS_H
#define CANIM_UTILS_H

#include "minitween.h"
#include "include/raylib/raylib.h"
#include "include/raylib/raymath.h"

#define SEGMENT_AMOUNT 3

typedef struct { MiniTween circle, tweens[SEGMENT_AMOUNT]; } WifiTweens;
typedef struct {
    Vector2 center;
    double angle[SEGMENT_AMOUNT];

    double ring_angle, angle_chip;

    double circle_radius, line_thickness;
    double line_gap;
    double resolution;
} WifiProperties;

static inline WifiTweens CreateWifi(double segment_duration, double prelay) {
    WifiTweens tween;
    tween.circle = MiniTweenCreate(segment_duration, 0.0, 0);
    tween.tweens[0] = MiniTweenCreate(segment_duration, -prelay * 2, &tween.circle);
    tween.tweens[1] = MiniTweenCreate(segment_duration, -prelay, &tween.tweens[0]);
    tween.tweens[2] = MiniTweenCreate(segment_duration, -prelay, &tween.tweens[1]);

    return tween;
}

static inline void DrawWifi(double t, const WifiTweens *tween, const WifiProperties *props) {
    double remapped;
    float angle = props->ring_angle;

    if(MiniTweenUpdate(&tween->circle, t, &remapped)) {
        Vector2 position = {0, Lerp(20, -2, remapped)};
        Color color      = {255, 255, 255, Lerp(0, 255, remapped)};
        DrawCircleV(position, props->circle_radius, color);
    } else if(remapped == tween->circle.duration) {
        DrawCircleV((Vector2){0, -2}, props->circle_radius, (Color){255, 255, 255, 255});
    }

    for(int i = 0; i < SEGMENT_AMOUNT; ++i) {
        Vector2 ring_lerp = {props->angle[i] - (angle - i * props->angle_chip), props->angle[i] - 180 + (angle - i * props->angle_chip)};
        Color color = WHITE;
        Vector2 thickness = {props->circle_radius + props->line_gap * (1 + i) + props->line_thickness * (1 + i * 2),
                             props->circle_radius + props->line_gap * (1 + i) + props->line_thickness * (2 + i * 2)};

        if(MiniTweenUpdate(&tween->tweens[i], t, &remapped)) {
            ring_lerp.x = Lerp(-90 + props->angle[i], ring_lerp.x, remapped), ring_lerp.y = Lerp(-90 + props->angle[i], ring_lerp.y, remapped);
            color.a = Lerp(0, 255, remapped);
        } else if(remapped < tween->tweens[1].duration)
            continue;

        DrawRing(props->center, thickness.x, thickness.y, ring_lerp.x, ring_lerp.y, props->resolution, color);
    }
}

static inline void DrawStripedLine(Vector2 start, Vector2 end, float thickness, float stripe_size, float offset, Color tint) {
    Vector2 difference_vector = (Vector2){end.x - start.x, end.y - start.y};
    float length = Vector2Length(difference_vector);

    int stripe_count = length / stripe_size / 2;
    float stripe_normalized = stripe_size / length;
    float offset_normalized = fmodf(offset, 2.0) * stripe_normalized;

    Vector2 tail_start = Vector2Lerp(start, end, fminf(stripe_count * 2 * stripe_normalized + offset_normalized, 1.0));
    Vector2 tail_end   = Vector2Lerp(start, end, fminf((stripe_count * 2 + 1) * stripe_normalized + offset_normalized, 1.0));

    DrawLineEx(start, Vector2Lerp(start, end, fmaxf(offset_normalized - stripe_normalized, 0.0)), thickness, tint);
    DrawLineEx(tail_start, tail_end, thickness, tint);

    for(int i = 0; i < stripe_count; ++i) {
        Vector2 new_start = Vector2Lerp(start, end, i * 2 * stripe_normalized + offset_normalized);
        Vector2 new_end   = Vector2Lerp(start, end, (i * 2 + 1) * stripe_normalized + offset_normalized);
        DrawLineEx(new_start, new_end, thickness, tint);
    }
}

#endif
