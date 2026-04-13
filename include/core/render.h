#ifndef CANIM_RENDER_H
#define CANIM_RENDER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct CanimRender CanimRender;

CanimRender* canimRenderStart(unsigned width, unsigned height, unsigned fps, bool output);

void canimRenderFinish(CanimRender* context);
void canimRenderSendFrame(CanimRender* context);
void canimRenderDraw(CanimRender* context, int x, int y, int width, int height);
void canimRenderBeginOutput(CanimRender* context);
void canimRenderEndOutput(CanimRender* context);

bool canimIsRendering(CanimRender* ctx);
unsigned canimGetFramerate(CanimRender* ctx);


#endif
