#ifndef CANIM_RL_RENDERTEXTURE_MSAA_H
#define CANIM_RL_RENDERTEXTURE_MSAA_H

#include "raylib/raylib.h"

typedef struct RenderTextureMSAA {
    RenderTexture render;
    Texture2D blit;
    unsigned blit_fbo;
} RenderTextureMSAA;

void InitMSAAInjector(void);

RenderTextureMSAA LoadRenderTextureMSAA(unsigned width, unsigned height, unsigned samples);
void DownsampleRenderTextureMSAA(RenderTextureMSAA render_texture);
void UnloadRenderTextureMSAA(RenderTextureMSAA render_texture);

// Unrelated to MSAA textures, needed to avoid using LoadImageFromTexture
void ExportDataTexture2D(Texture2D texture, void* output);

#endif
