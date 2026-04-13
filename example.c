#if 0
gcc test.c -o project.so -shared -fPIC -O0
return
#endif

#include "include/raylib/raylib.h"
#include "include/raylib/raymath.h"
#include "include/canim.h"

Camera2D camera;

void animationTwo(double, double);

void canimSetup(void) {
    camera = (Camera2D){.zoom = 1.0, .offset = {canimGetRenderWidth() / 2., canimGetRenderHeight() / 2. }};
    canimAddAnimation(0.0, 5, animationTwo);

    SetConfigFlags(FLAG_MSAA_4X_HINT);
}

void animationTwo(double t, double actual) {
BeginMode2D(camera);
    DrawRing(Vector2Zero(), 0, 20, 0 - 45, -180 + 45, 10, WHITE);
    DrawRing(Vector2Zero(), 28, 40, 0 - 40, -180 + 40, 10, WHITE);
    DrawRing(Vector2Zero(), 48, 60, 0 - 40, -180 + 40, 10, WHITE);
EndMode2D();
}
