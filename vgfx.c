#include "vgfx.h"
#include <SDL3/SDL.h>
#include <stdlib.h>
#include <math.h>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static uint32_t window_flags;
static bool _window_should_close;
static float background_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
const bool *keys;

void iVG_ColorSet(float *color);
void iVG_KeysUpdate();
SDL_FPoint iVG_FPoint2(float* point);
SDL_FColor iVG_FColor4(float* color);
SDL_Vertex iVG_Vertex(float* point, float* color, float* texcoord);
SDL_Vertex iVG_VertexColored(float* point, float* color);

void VG_GetBackgroundColor(float* out) {
    VRGBA_Copy(out, background_color);
}

void VG_SetBackgroundColor(float* in) {
    VRGBA_Copy(background_color, in);
}

void VG_PollEvents() {
    iVG_KeysUpdate();
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
	switch (event.type) {
	    case SDL_EVENT_QUIT:
		_window_should_close = true;
	    }
    }
}

void VG_InitWindow(char* name, float* size, uint32_t flags) {
    SDL_SetAppMetadata(name, "0.1", name);

    if (!SDL_Init(SDL_INIT_VIDEO)) {
	SDL_Log("Couldn't initialize SDL video: %s", SDL_GetError());
	exit(1);
    }
    
    if (!SDL_CreateWindowAndRenderer(name, size[0], size[1], flags, &window, &renderer)) {
	SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
    }

    VG_SetFlags(flags);
    VG_UpdateWindowSize();
}

void VG_GetWindowSize(float* out) {
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    out[0] = w;
    out[1] = h;
}

void VG_UpdateWindowSize() {
    float size[2];
    VG_GetWindowSize(size);
    SDL_SetRenderLogicalPresentation(renderer, size[0], size[1], SDL_LOGICAL_PRESENTATION_LETTERBOX);
}

void VG_SetFlags(uint32_t flags) {
    window_flags = flags;
    if (window_flags & VG_WINDOW_FLAG_VSYNC) {
	SDL_SetRenderVSync(renderer, SDL_RENDERER_VSYNC_ADAPTIVE);
    }    
}

bool VG_WindowShouldClose() {
    return _window_should_close;
}

bool VG_IsKeyPressed(uint64_t key) {
    
}

bool VG_IsKeyDown(uint64_t key);

void VG_DrawingBegin() {
    VG_UpdateWindowSize();
    VG_PollEvents();
    VG_ClearScreen(background_color);
}

void VG_DrawingEnd() {
    VG_RenderFlush();
}

SDL_FRect iVG_GetFRect(float* pos, float* size) {
    return (SDL_FRect){.x = pos[0], .y = pos[1], .w = size[0], .h = size[1]};
}

void VG_DrawRect(float* pos, float* size, float* color);

void VG_FillRect(float* pos, float* size, float* color) {
    SDL_FRect rect = iVG_GetFRect(pos, size);
    iVG_SetColor(color);
    SDL_RenderFillRect(renderer, &rect);
}

void VG_FillRectCentered(float* pos, float* size, float* color) {
    float realpos[2];
    float half[2];
    VM2_ScaleO(size, 0.5, half);
    VM2_SubO(pos, half, realpos);
    VG_FillRect(realpos, size, color);
}

void VG_DrawCircle(float* pos, float r, float* color);

void VG_FillPolygon(float *pos, float r, float angle, uint32_t sides, float* color) {
    SDL_Vertex verts[3*sides];
    float prev[2], now[2];
    prev[0] = pos[0] + cosf(angle - 2*3.1415926535f/sides) * r;
    prev[1] = pos[1] + sing(angle - 2*3.1415926535f/sides) * r;
    float center[2];
    VM2_Copy(center, pos);
    
    for (int i = 0; i < sides; i++) {
	now[0] = pos[0] + cosf(angle) * r;
	bow[0] = pos[1] + sinf(angle) * r;
	verts[3*i]   = iVG_VertexColored(now,    color);
	verts[3*i+1] = iVG_VertexColored(center, color);
	verts[3*i+2] = iVG_VertexColored(prev,   color);
	angle += 2*3.1415926535f/sides;
	prevx = x; prevy = y;
    }

    iVG_ColorSet(color);
    SDL_RenderGeometry(renderer, NULL, verts, ARRLEN(verts), NULL, 0);
}

void VG_FillCircle(float *pos, float r, float* color) {
    VG_FillPolygon(pos, r, 0, 32, color);
}

void VG_ClearBackground() {
    VG_ClearScreen(background_color);
}

void iVG_ColorSet(float *color) {
    SDL_SetRenderDrawColorFloat(renderer, color[0], color[1], color[2], color[3]);
}

void VG_ClearScreen(float* color) {
    iVG_ColorSet(color);
    SDL_RenderClear(renderer);    
}

void VG_RenderFlush() {
    SDL_RenderPresent(renderer);
}

//const bool *VG_GetKeys(int *number_of_keys) {
//    return SDL_GetKeyboardState(number_of_keys);
//}

void iVG_KeysUpdate() {
    keys = VG_GetKeys(NULL);
}

SDL_FPoint iVG_FPoint2(float* point) {
    return (SDL_FPoint){ point[0], point[1] };
}

SDL_FColor iVG_FColor4(float* color) {
    return (SDL_FColor){ color[0], color[1], color[2], color[3] };
}

SDL_Vertex iVG_Vertex(float* point, float* color, float* tex_coord) {
    SDL_Vertex vertex;
    vertex.position = iVG_FPoint2(point);
    vertex.color = iVG_FColor4(color);
    vertex.tex_coord = iVG_FPoint2(tex_coord);
}

SDL_Vertex iVG_VertexColored(float* point, float* color) {
    iVG_Vertex(point, color, VM2_ZERO);
}
