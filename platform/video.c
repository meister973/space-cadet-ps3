/*
 * RSX output for a software-rendered game: each frame the game's screen is copied into a
 * texture and drawn as textured quads. Display setup, flips and the command ring follow the
 * Balatro port's gfx_device.c, which is proven on RPCS3 and hardware.
 */
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <rsx/rsx.h>
#include <rsx/gcm_sys.h>
#include <sysutil/video.h>

#include "pixels.h"
#include "quad_fpo.h"
#include "quad_vpo.h"
#include "rsx_ring.h"
#include "video.h"

#define HOST_SIZE          (4 * 1024 * 1024)
#define INITIAL_CB_SIZE    0x80000
#define RING_OFFSET        0x100000
#define RING_SIZE          (1 * 1024 * 1024)
#define BUFFER_COUNT       2
#define MAX_QUADS          16
#define VERTICES_PER_QUAD  6
#define LABEL_INDEX        255

typedef struct {
    float x, y, u, v;
} QuadVertex;

typedef struct {
    unsigned char *pixels;
    u32 offset;
    u32 pitch;
} ScreenTexture;

static gcmContextData *context;
static u32 screen_width;
static u32 screen_height;
static u32 screen_pitch;
static u32 color_offset[BUFFER_COUNT];
static void *color_pixels[BUFFER_COUNT];
static u32 depth_offset;
static u32 frame_index;
static int first_flip = 1;

static ScreenTexture textures[BUFFER_COUNT];
static u32 texture_width;
static u32 texture_height;

static QuadVertex *vertex_memory;
static u32 vertex_offset;

static rsxVertexProgram *vertex_program;
static rsxFragmentProgram *fragment_program;
static void *vertex_ucode;
static u32 fragment_offset;
static rsxProgramConst *projection_constant;
static u32 label_value = 1;

static int configure_display(void)
{
    videoState state;
    videoResolution resolution;
    videoConfiguration config;

    if (videoGetState(0, 0, &state) != 0)
        return -1;
    /* 720p when the TV offers it: the table is 416 lines tall, so 1080p adds nothing. */
    u8 mode = state.displayMode.resolution;
    if (videoGetResolutionAvailability(0, VIDEO_RESOLUTION_720, VIDEO_ASPECT_AUTO, 0) == 1)
        mode = VIDEO_RESOLUTION_720;
    if (videoGetResolution(mode, &resolution) != 0)
        return -1;

    screen_width = resolution.width;
    screen_height = resolution.height;
    screen_pitch = (screen_width * 4 + 63) & ~63u;

    memset(&config, 0, sizeof(config));
    config.resolution = mode;
    config.format = VIDEO_BUFFER_FORMAT_XRGB;
    config.aspect = state.displayMode.aspect;
    config.pitch = screen_pitch;
    return videoConfigure(0, &config, NULL, 0);
}

static int allocate_surfaces(void)
{
    for (u32 i = 0; i < BUFFER_COUNT; i++) {
        color_pixels[i] = rsxMemalign(64, screen_pitch * screen_height);
        if (color_pixels[i] == NULL || rsxAddressToOffset(color_pixels[i], &color_offset[i]) != 0)
            return -1;
        if (gcmSetDisplayBuffer((u8)i, color_offset[i], screen_pitch, screen_width, screen_height) != 0)
            return -1;
    }

    void *depth = rsxMemalign(64, screen_pitch * screen_height);
    if (depth == NULL)
        return -1;
    return rsxAddressToOffset(depth, &depth_offset);
}

static int allocate_vertices(void)
{
    vertex_memory = rsxMemalign(128, sizeof(QuadVertex) * MAX_QUADS * VERTICES_PER_QUAD * BUFFER_COUNT);
    if (vertex_memory == NULL)
        return -1;
    return rsxAddressToOffset(vertex_memory, &vertex_offset);
}

static int open_program(void)
{
    u32 vertex_size;
    u32 fragment_size;
    void *fragment_ucode;

    vertex_program = (rsxVertexProgram *)quad_vpo;
    fragment_program = (rsxFragmentProgram *)quad_fpo;
    rsxVertexProgramGetUCode(vertex_program, &vertex_ucode, &vertex_size);
    projection_constant = rsxVertexProgramGetConst(vertex_program, "projection");

    rsxFragmentProgramGetUCode(fragment_program, &fragment_ucode, &fragment_size);
    void *fragment_memory = rsxMemalign(64, fragment_size);
    if (fragment_memory == NULL || projection_constant == NULL)
        return -1;
    memcpy(fragment_memory, fragment_ucode, fragment_size);
    return rsxAddressToOffset(fragment_memory, &fragment_offset);
}

/* Blend state must follow every surface bind, or the RSX ignores it. */
static void set_fixed_state(void)
{
    rsxSetColorMask(context, GCM_COLOR_MASK_B | GCM_COLOR_MASK_G | GCM_COLOR_MASK_R | GCM_COLOR_MASK_A);
    rsxSetColorMaskMrt(context, 0);
    rsxSetDepthTestEnable(context, GCM_FALSE);
    rsxSetDepthWriteEnable(context, GCM_FALSE);
    rsxSetCullFaceEnable(context, GCM_FALSE);
    rsxSetShadeModel(context, GCM_SHADE_MODEL_SMOOTH);
    rsxSetBlendEnable(context, GCM_FALSE);
}

static void bind_back_buffer(void)
{
    gcmSurface surface;
    f32 scale[4] = { screen_width * 0.5f, screen_height * -0.5f, 0.5f, 0.0f };
    f32 bias[4] = { screen_width * 0.5f, screen_height * 0.5f, 0.5f, 0.0f };

    memset(&surface, 0, sizeof(surface));
    surface.type = GCM_SURFACE_TYPE_LINEAR;
    surface.antiAlias = GCM_SURFACE_CENTER_1;
    surface.colorFormat = GCM_SURFACE_X8R8G8B8;
    surface.colorTarget = GCM_SURFACE_TARGET_0;
    surface.colorLocation[0] = GCM_LOCATION_RSX;
    surface.colorOffset[0] = color_offset[frame_index];
    surface.colorPitch[0] = screen_pitch;
    for (u32 i = 1; i < GCM_MAX_MRT_COUNT; i++) {
        surface.colorLocation[i] = GCM_LOCATION_RSX;
        surface.colorPitch[i] = 64;
    }
    surface.depthFormat = GCM_SURFACE_ZETA_Z24S8;
    surface.depthLocation = GCM_LOCATION_RSX;
    surface.depthOffset = depth_offset;
    surface.depthPitch = screen_pitch;
    surface.width = screen_width;
    surface.height = screen_height;

    rsxSetSurface(context, &surface);
    set_fixed_state();
    rsxSetViewport(context, 0, 0, screen_width, screen_height, 0.0f, 1.0f, scale, bias);
    rsxSetScissor(context, 0, 0, screen_width, screen_height);
    for (u32 i = 0; i < 8; i++)
        rsxSetViewportClip(context, i, screen_width, screen_height);
}

static void bind_program(void)
{
    f32 projection[4] __attribute__((aligned(16))) = {
        2.0f / screen_width, -2.0f / screen_height, -1.0f, 1.0f
    };

    rsxLoadVertexProgram(context, vertex_program, vertex_ucode);
    rsxSetVertexProgramParameter(context, vertex_program, projection_constant, projection);
    rsxLoadFragmentProgramLocation(context, fragment_program, fragment_offset, GCM_LOCATION_RSX);
}

static void bind_texture(const ScreenTexture *texture)
{
    gcmTexture description;

    rsxInvalidateTextureCache(context, GCM_INVALIDATE_TEXTURE);
    description.format = GCM_TEXTURE_FORMAT_A8R8G8B8 | GCM_TEXTURE_FORMAT_LIN;
    description.mipmap = 1;
    description.dimension = GCM_TEXTURE_DIMS_2D;
    description.cubemap = GCM_FALSE;
    description.remap = (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_B_SHIFT) |
                        (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_G_SHIFT) |
                        (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_R_SHIFT) |
                        (GCM_TEXTURE_REMAP_TYPE_REMAP << GCM_TEXTURE_REMAP_TYPE_A_SHIFT) |
                        (GCM_TEXTURE_REMAP_COLOR_B << GCM_TEXTURE_REMAP_COLOR_B_SHIFT) |
                        (GCM_TEXTURE_REMAP_COLOR_G << GCM_TEXTURE_REMAP_COLOR_G_SHIFT) |
                        (GCM_TEXTURE_REMAP_COLOR_R << GCM_TEXTURE_REMAP_COLOR_R_SHIFT) |
                        (GCM_TEXTURE_REMAP_COLOR_A << GCM_TEXTURE_REMAP_COLOR_A_SHIFT);
    description.width = texture_width;
    description.height = texture_height;
    description.depth = 1;
    description.location = GCM_LOCATION_RSX;
    description.pitch = texture->pitch;
    description.offset = texture->offset;

    rsxLoadTexture(context, 0, &description);
    rsxTextureControl(context, 0, GCM_TRUE, 0 << 8, 12 << 8, GCM_TEXTURE_MAX_ANISO_1);
    rsxTextureFilter(context, 0, 0, GCM_TEXTURE_LINEAR, GCM_TEXTURE_LINEAR, GCM_TEXTURE_CONVOLUTION_QUINCUNX);
    rsxTextureWrapMode(context, 0, GCM_TEXTURE_CLAMP_TO_EDGE, GCM_TEXTURE_CLAMP_TO_EDGE,
                       GCM_TEXTURE_CLAMP_TO_EDGE, 0, GCM_TEXTURE_ZFUNC_LESS, 0);
}

static void wait_for_rsx(void)
{
    rsxSetWriteBackendLabel(context, LABEL_INDEX, label_value);
    rsxFlushBuffer(context);
    while (*(vu32 *)gcmGetLabelAddress(LABEL_INDEX) != label_value)
        usleep(30);
    label_value++;
}

static void wait_previous_flip(void)
{
    if (first_flip) {
        gcmResetFlipStatus();
        first_flip = 0;
        return;
    }
    while (gcmGetFlipStatus() != 0)
        usleep(200);
    gcmResetFlipStatus();
}

int video_open(void)
{
    void *host = memalign(1024 * 1024, HOST_SIZE);

    if (host == NULL || rsxInit(&context, INITIAL_CB_SIZE, HOST_SIZE, host) != 0)
        return -1;
    if (rsx_ring_attach(context, (char *)host + RING_OFFSET, RING_SIZE) != 0)
        return -1;
    if (configure_display() != 0 || allocate_surfaces() != 0 || allocate_vertices() != 0)
        return -1;
    if (open_program() != 0)
        return -1;

    gcmSetFlipMode(GCM_FLIP_VSYNC);
    bind_back_buffer();
    return 0;
}

int video_create_screen(int width, int height)
{
    texture_width = (u32)width;
    texture_height = (u32)height;

    for (u32 i = 0; i < BUFFER_COUNT; i++) {
        textures[i].pitch = (texture_width * 4 + 63) & ~63u;
        textures[i].pixels = rsxMemalign(128, textures[i].pitch * texture_height);
        if (textures[i].pixels == NULL || rsxAddressToOffset(textures[i].pixels, &textures[i].offset) != 0)
            return -1;
        memset(textures[i].pixels, 0, textures[i].pitch * texture_height);
    }
    return 0;
}

/* Without this, the RSX keeps reading freed memory after the process leaves. */
void video_close(void)
{
    if (context == NULL)
        return;
    gcmSetWaitFlip(context);
    wait_for_rsx();
}

int video_width(void) { return (int)screen_width; }
int video_height(void) { return (int)screen_height; }

/* The texture of two frames ago: that frame's flip finished before the last present returned. */
void video_upload_screen(const unsigned char *rgba, size_t pitch)
{
    ScreenTexture *texture = &textures[frame_index];

    pixels_copy_rgba_to_argb(rgba, pitch, texture->pixels, texture->pitch, (int)texture_width, (int)texture_height);
}

static void write_quad(QuadVertex *target, const VideoQuad *quad)
{
    const float left = quad->x, right = quad->x + quad->width;
    const float top = quad->y, bottom = quad->y + quad->height;
    const QuadVertex corners[VERTICES_PER_QUAD] = {
        { left, top, quad->u0, quad->v0 }, { right, top, quad->u1, quad->v0 },
        { left, bottom, quad->u0, quad->v1 }, { right, top, quad->u1, quad->v0 },
        { right, bottom, quad->u1, quad->v1 }, { left, bottom, quad->u0, quad->v1 },
    };

    memcpy(target, corners, sizeof(corners));
}

void video_draw(const VideoQuad *quads, int count)
{
    u32 first = frame_index * MAX_QUADS * VERTICES_PER_QUAD;
    u32 base = vertex_offset + first * sizeof(QuadVertex);

    if (count > MAX_QUADS)
        count = MAX_QUADS;
    for (int i = 0; i < count; i++)
        write_quad(vertex_memory + first + i * VERTICES_PER_QUAD, &quads[i]);

    rsx_ring_make_room(context);
    rsxSetClearColor(context, 0);
    rsxClearSurface(context, GCM_CLEAR_R | GCM_CLEAR_G | GCM_CLEAR_B | GCM_CLEAR_A);
    bind_program();
    bind_texture(&textures[frame_index]);
    rsxBindVertexArrayAttrib(context, GCM_VERTEX_ATTRIB_POS, 0, base, sizeof(QuadVertex), 2,
                             GCM_VERTEX_DATA_TYPE_F32, GCM_LOCATION_RSX);
    rsxBindVertexArrayAttrib(context, GCM_VERTEX_ATTRIB_TEX0, 0, base + 2 * sizeof(float), sizeof(QuadVertex), 2,
                             GCM_VERTEX_DATA_TYPE_F32, GCM_LOCATION_RSX);
    rsxInvalidateVertexCache(context);
    rsxDrawVertexArray(context, GCM_TYPE_TRIANGLES, 0, (u32)count * VERTICES_PER_QUAD);
}

void video_present(void)
{
    rsx_ring_make_room(context);
    wait_previous_flip();
    gcmSetFlip(context, (u8)frame_index);
    rsxFlushBuffer(context);
    gcmSetWaitFlip(context);

    frame_index ^= 1;
    bind_back_buffer();
}

int video_dump_frame(const char *path)
{
    u32 shown = frame_index ^ 1;

    wait_for_rsx();
    FILE *file = fopen(path, "wb");
    if (file == NULL)
        return -1;

    unsigned char *row = malloc(screen_width * 3);
    if (row == NULL) {
        fclose(file);
        return -1;
    }

    fprintf(file, "P6\n%u %u\n255\n", screen_width, screen_height);
    for (u32 y = 0; y < screen_height; y++) {
        const unsigned char *source = (const unsigned char *)color_pixels[shown] + y * screen_pitch;

        for (u32 x = 0; x < screen_width; x++) {
            row[x * 3] = source[x * 4 + 1];
            row[x * 3 + 1] = source[x * 4 + 2];
            row[x * 3 + 2] = source[x * 4 + 3];
        }
        fwrite(row, 1, screen_width * 3, file);
    }

    free(row);
    fclose(file);
    return 0;
}
