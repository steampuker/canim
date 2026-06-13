#include "./rl_rendertexture_msaa.h"
#include "raylib/raylib.h"
#include "raylib/rlgl.h"

#include <stdint.h>

#define GL_PROC_ADDRESS(x) rlGetProcAddress(x)

enum { GL_READ_FRAMEBUFFER = 0x8CA8, GL_DRAW_FRAMEBUFFER = 0x8CA9, GL_COLOR_BUFFER_BIT = 0x00004000, GL_NEAREST = 0x2600,
       GL_TEXTURE_2D = 0x0DE1, GL_TEXTURE_2D_MULTISAMPLE = 0x9100,
       GL_RENDERBUFFER = 0x8D41, GL_DEPTH_COMPONENT = 0x1902, GL_DEPTH_ATTACHMENT = 0x8D00,
       GL_RGBA = 0x1908, GL_RGBA8 = 0x8058, GL_RGB8 = 0x805,
       GL_FRAMEBUFFER = 0x8D40, GL_COLOR_ATTACHMENT0 = 0x8CE0,
       GL_UNPACK_ALIGNMENT = 0x0CF5, GL_PACK_ALIGNMENT = 0x0D05, GL_UNSIGNED_BYTE = 0x1401 };

typedef void (*bindFramebufferPFN)(uint32_t, uint32_t);
typedef void (*blitFramebufferPFN)(int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, int32_t, uint32_t, uint32_t);
typedef void (*framebufferTexture2DPFN)(uint32_t, uint32_t, uint32_t, uint32_t, int32_t);
typedef void (*framebufferRenderbufferPFN)(uint32_t, uint32_t, uint32_t, uint32_t);

typedef void (*genTexturesPFN)(int32_t, uint32_t*);
typedef void (*bindTexturePFN)(uint32_t, uint32_t);
typedef void (*texImage2DPFN)(uint32_t target, int32_t level, int32_t internal_format, int32_t width, int32_t height, int32_t border, uint32_t format, uint32_t type, const void* data);
typedef void (*texStorage2DMultisamplePFN)(uint32_t, int32_t, uint32_t, int32_t, int32_t, uint8_t);
typedef void (*pixelStoreiPFN)(uint32_t, int32_t);
typedef void (*readPixelsPFN)(int32_t, int32_t, int32_t, int32_t, uint32_t, uint32_t, void*);
typedef void (*getTexImagePFN)(uint32_t, int32_t, uint32_t, uint32_t, void*);

//typedef void (*genRenderbuffersPFN)(int32_t, uint32_t*);
//typedef void (*bindRenderbufferPFN)(uint32_t, uint32_t*);
typedef void (*renderbufferStorageMultisamplePFN)(uint32_t, int32_t, uint32_t, int32_t, int32_t);

static struct {
    bindFramebufferPFN bindFramebuffer;
    blitFramebufferPFN blitFramebuffer;
    framebufferTexture2DPFN framebufferTexture2D;
    framebufferRenderbufferPFN framebufferRenderbuffer;

    genTexturesPFN genTextures, genRenderbuffers;
    bindTexturePFN bindTexture, bindRenderbuffer;
    texImage2DPFN texImage2D;
    texStorage2DMultisamplePFN texStorage2DMultisample;
    renderbufferStorageMultisamplePFN renderbufferStorageMultisample;

    pixelStoreiPFN pixelStorei;
    readPixelsPFN readPixels;
    getTexImagePFN getTexImage;

    int version;
} gl;

void InitMSAAInjector(void)
{
    gl.bindFramebuffer         = (bindFramebufferPFN)GL_PROC_ADDRESS("glBindFramebuffer");
    gl.blitFramebuffer         = (blitFramebufferPFN)GL_PROC_ADDRESS("glBlitFramebuffer");
    gl.framebufferTexture2D    = (framebufferTexture2DPFN)GL_PROC_ADDRESS("glFramebufferTexture2D");
    gl.framebufferRenderbuffer = (framebufferRenderbufferPFN)GL_PROC_ADDRESS("glFramebufferRenderbuffer");

    gl.genTextures           = (genTexturesPFN)GL_PROC_ADDRESS("glGenTextures");
    gl.bindTexture           = (bindTexturePFN)GL_PROC_ADDRESS("glBindTexture");
    gl.texImage2D            = (texImage2DPFN)GL_PROC_ADDRESS("glTexImage2D");

    gl.genRenderbuffers      = (genTexturesPFN)GL_PROC_ADDRESS("glGenRenderbuffers");
    gl.bindRenderbuffer      = (bindTexturePFN)GL_PROC_ADDRESS("glBindRenderbuffer");

    gl.texStorage2DMultisample = (texStorage2DMultisamplePFN)GL_PROC_ADDRESS("glTexStorage2DMultisample");
    gl.renderbufferStorageMultisample = (renderbufferStorageMultisamplePFN)GL_PROC_ADDRESS("glRenderbufferStorageMultisample");

    if(!gl.texStorage2DMultisample) // Fallback to glTexImage2DMultisample
        gl.texStorage2DMultisample = (texStorage2DMultisamplePFN)GL_PROC_ADDRESS("glTexImage2DMultisample");

    gl.pixelStorei           = (pixelStoreiPFN)GL_PROC_ADDRESS("glPixelStorei");
    gl.readPixels            = (readPixelsPFN)GL_PROC_ADDRESS("glReadPixels");
    gl.getTexImage           = (getTexImagePFN)GL_PROC_ADDRESS("glGetTexImage");

    gl.version = rlGetVersion();
}

RenderTextureMSAA LoadRenderTextureMSAA(unsigned width, unsigned height, unsigned samples)
{
    RenderTextureMSAA target = {0};

    target.render.id = rlLoadFramebuffer();
    target.blit_fbo = rlLoadFramebuffer();
    if (target.render.id <= 0 || target.blit_fbo <= 0)
        return TRACELOG(LOG_WARNING, "FBO: Framebuffer object can not be created"), target;

    Texture2D texture = {.width = width, .height = height, .format = RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, .mipmaps = 1};
    Texture2D depth_texture = {.width = width, .height = height, .mipmaps = 1};

    gl.pixelStorei(GL_UNPACK_ALIGNMENT, 1);
    gl.genTextures(1, &texture.id);
    gl.bindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture.id);
    gl.texStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA8, width, height, 1);
    gl.bindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

    gl.genRenderbuffers(1, &depth_texture.id);
    gl.bindRenderbuffer(GL_RENDERBUFFER, depth_texture.id);
    gl.renderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH_COMPONENT, width, height);

    gl.bindRenderbuffer(GL_RENDERBUFFER, 0);

    target.blit.id = rlLoadTexture(0, width, height, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8, 1);
    target.blit_depth.id = rlLoadTextureDepth(width, height, true);

    target.blit_depth.width = target.blit.width = width;
    target.blit_depth.height = target.blit.height = height;
    target.blit_depth.mipmaps = target.blit.mipmaps = 1;
    target.blit.format = RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    target.blit_depth.format = 19;

    target.render.texture = texture;
    target.render.depth = depth_texture;

    gl.bindFramebuffer(GL_FRAMEBUFFER, target.render.id);

    gl.framebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, texture.id, 0);
    gl.framebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_texture.id);

    gl.bindFramebuffer(GL_FRAMEBUFFER, target.blit_fbo);

    gl.framebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.blit.id, 0);
    gl.framebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, target.blit_depth.id);

    gl.bindFramebuffer(GL_FRAMEBUFFER, 0);

    rlTextureParameters(target.render.texture.id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_REPEAT);
    rlTextureParameters(target.render.texture.id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_REPEAT);

    rlTextureParameters(target.blit.id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_REPEAT);
    rlTextureParameters(target.blit.id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_REPEAT);

    if (rlFramebufferComplete(target.render.id) && rlFramebufferComplete(target.blit_fbo))
        TRACELOG(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", target.id);

    return target;
}

void DownsampleRenderTextureMSAA(RenderTextureMSAA render_texture)
{
    /* Do we need to flush the batch? */
    rlDrawRenderBatchActive();
    int current_fbo = rlGetActiveFramebuffer();

    int width  = render_texture.render.texture.width,
        height = render_texture.render.texture.height;
    gl.bindFramebuffer(GL_READ_FRAMEBUFFER, render_texture.render.id);
    gl.bindFramebuffer(GL_DRAW_FRAMEBUFFER, render_texture.blit_fbo);
    gl.blitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    gl.bindFramebuffer(GL_FRAMEBUFFER, current_fbo);
}

void UnloadRenderTextureMSAA(RenderTextureMSAA render_texture)
{
    if(render_texture.blit.id)
        rlUnloadTexture(render_texture.blit.id);

    if(render_texture.render.texture.id)
        rlUnloadTexture(render_texture.render.texture.id);

    rlUnloadFramebuffer(render_texture.render.id);
    rlUnloadFramebuffer(render_texture.blit_fbo);
}

void ExportDataTexture2D(Texture2D texture, void* output)
{
    if(!output)
        return TraceLog(LOG_ERROR, "Null pointer passed to ExportDataTexture2D");

    if(texture.format <= 0 || texture.format >= RL_PIXELFORMAT_COMPRESSED_DXT1_RGB)
        return TRACELOG(RL_LOG_WARNING, "ExportDataTexture2D: [ID %i] Data retrieval not suported for pixel format (%i)", texture.id, texture.format);

    //if(gl.version >= RL_OPENGL_ES_20 ) {
    //    unsigned int fboId = rlLoadFramebuffer();

    //    gl.bindFramebuffer(GL_FRAMEBUFFER, fboId);
    //    gl.bindTexture(GL_TEXTURE_2D, 0);

    //    // Attach our texture to FBO
    //    gl.framebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture.id, 0);

    //    // We read data as RGBA because FBO texture is configured as RGBA, despite binding another texture format
    //    //pixels = (unsigned char *)RL_MALLOC(rlGetPixelDataSize(width, height, RL_PIXELFORMAT_UNCOMPRESSED_R8G8B8A8));
    //    gl.readPixels(0, 0, texture.width, texture.height, GL_RGBA, GL_UNSIGNED_BYTE, output);

    //    gl.bindFramebuffer(GL_FRAMEBUFFER, 0);
    //    rlUnloadFramebuffer(fboId);
    //    return;
    //}

    unsigned internal, format, type;
    rlGetGlTextureFormats(texture.format, &internal, &format, &type);

    gl.bindTexture(GL_TEXTURE_2D, texture.id);
    gl.pixelStorei(GL_PACK_ALIGNMENT, 1);
    gl.getTexImage(GL_TEXTURE_2D, 0, format, type, output);
    gl.bindTexture(GL_TEXTURE_2D, 0);
}
