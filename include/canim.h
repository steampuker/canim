#ifndef CANIM_LIB_H
#define CANIM_LIB_H

void canimSetup(void);

void canimAddAnimation(double start, double end, void (*callback)(double progress, double actual));
void canimAddAnimationManaged(double start, double end, void (*callback)(double progress, double actual), void (*init)(void), void (*deinit)(void));
void canimSetTotalLength(double seconds);
double canimGetTotalLength(void);

unsigned canimGetRenderWidth(void);
unsigned canimGetRenderHeight(void);
unsigned canimGetRenderFPS(void);

void canimSetRenderWidth(double);
void canimSetRenderHeight(double);
void canimSetRenderFPS(double);

#endif
