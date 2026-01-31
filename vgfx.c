#include "vgfx.h"
#include <SDL3/SDL.h>
#include <stdlib.h>

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static uint32_t window_flags;
static bool _window_should_close;
static float background_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};


void iVG_SetColor(float *color);

void VG_GetBackgroundColor(float* out) {
    VM2_Copy(out, background_color);
}

void VG_SetBackgroundColor(float* in) {
    VM2_Copy(background_color, out);
}

void VG_PollEvents() {
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

bool VG_IsKeyPressed(uint64_t key);

bool VG_IsKeyDown(uint64_t key);

uint64_t *VG_GetKeys();

void VG_DrawingBegin() {
    VG_UpdateWindowSize();
    VG_PollEvents();
    VG_ClearScreen(background_color);
}

void VG_DrawingEnd() {
    VG_RenderFlush();
}

void VG_DrawRect(float* pos, float* size, float* color);

void VG_FillRect(float* pos, float* size, float* color);

void VG_DrawCircle(float* pos, float r, float* color);

void VG_FillCircle(float* pos, float r, float* color);

void VG_ClearBackground() {
    VG_ClearScreen(background_color);
}

void iVG_SetColor(float *color) {
    SDL_SetRenderDrawColorFloat(renderer, color[0], color[1], color[2], color[3]);
}

void VG_ClearScreen(float* color) {
    iVG_SetColor(color);
    SDL_RenderClear(renderer);    
}

void VG_RenderFlush() {
    SDL_RenderPresent(renderer);
}

