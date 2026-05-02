#include "include/raylib/raylib.h"
#include "include/raylib/raymath.h"
#include "include/canim.h"

void animation(double t, double actual)
{
    const Vector2 center = { canimGetRenderWidth() / 2., canimGetRenderHeight() / 2. };
    DrawRing(center, 0, 100, 0, 360 * t, 8 + 32 * t, WHITE);
}

void canimSetup()
{
    canimAddAnimation(0, 5, animation);
}
