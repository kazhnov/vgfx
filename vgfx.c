#include "vgfx.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <GL/gl.h>
#include <stdlib.h>
#include <math.h>

#define ARRLEN(x) ((sizeof(x))/(sizeof(x[0])))


static SDL_Window *window = NULL;
//static SDL_Renderer *renderer = NULL;
static SDL_GLContext glcontext = NULL;
static uint32_t window_flags;
static bool _window_should_close;
static float background_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
const uint8_t *keys;
static float window_size[2];

void iVG_ColorSet(float *color);
void iVG_KeysUpdate();
SDL_FPoint iVG_FPoint2(float* point);
SDL_Color  iVG_Color4(float* color);
SDL_Vertex iVG_Vertex(float* point, float* color, float* texcoord);
SDL_Vertex iVG_VertexColored(float* point, float* color);
void iVG_SetupOpengl();
void iVG_GLFillRect(float* pos, float* size, float* color);

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
	    case SDL_QUIT:
		_window_should_close = true;
	    }
    }
}

void VG_InitWindow(char* name, float* size, uint32_t flags) {
//    SDL_SetAppMetadata(name, "0.1", name);
    SDL_SetWindowTitle(window, name);
    VM2_Copy(window_size, size);
    

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
	SDL_Log("Couldn't initialize SDL video: %s", SDL_GetError());
	exit(1);
    }

    window = SDL_CreateWindow(name,
		     SDL_WINDOWPOS_UNDEFINED,
		     SDL_WINDOWPOS_UNDEFINED,
		     size[0], size[1],
		     flags | SDL_WINDOW_SHOWN);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    
    if (!window) {
	SDL_Log("Couldn't create window: %s", SDL_GetError());
    }


    glcontext = SDL_GL_CreateContext(window);
    
    VG_SetFlags(flags);
    VG_UpdateWindowSize();
    iVG_SetupOpengl();
}

void VG_GetWindowSize(float* out) {
    VM2_Copy(out, window_size);
}

void VG_UpdateWindowSize() {
    VG_GetWindowSize(window_size);
    glViewport(0, 0, window_size[0], window_size[1]);
//    SDL_SetRenderLogicalPresentation(renderer, size[0], size[1], SDL_LOGICAL_PRESENTATION_LETTERBOX);    
}

void VG_SetFlags(uint32_t flags) {
    window_flags = flags;
    if (window_flags & VG_WINDOW_FLAG_VSYNC) {
//	SDL_SetRenderVSync(renderer, SDL_RENDERER_VSYNC_ADAPTIVE);
	SDL_GL_SetSwapInterval(1);
    }    
}

bool VG_WindowShouldClose() {
    return _window_should_close;
}

bool VG_IsKeyPressed(uint64_t key) {
    return keys[key];
}

bool VG_IsKeyDown(uint64_t key) {
    return keys[key];
}

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

void VG_FillRect(float* pos, float* size, float* color) {
    SDL_FRect rect = iVG_GetFRect(pos, size);
    iVG_ColorSet(color);
//    SDL_RenderFillRect(renderer, &rect);
    iVG_GLFillRect(pos, size, color);
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
    if (sides < 3) return;
    float center[2];
    VM2_Copy(center, pos);

    SDL_Vertex verts[3*sides];
    float prev[2], now[2];
    
    prev[0] = center[0] + cosf(angle - 2*3.1415926535f/sides) * r;
    prev[1] = center[1] + sinf(angle - 2*3.1415926535f/sides) * r;
    
    for (int i = 0; i < sides; i++) {
	now[0] = center[0] + cosf(angle) * r;
	now[1] = center[1] + sinf(angle) * r;
	verts[3*i]   = iVG_VertexColored(now,    color);
	verts[3*i+1] = iVG_VertexColored(center, color);
	verts[3*i+2] = iVG_VertexColored(prev,   color);
	angle += 2*3.1415926535f/sides;
	VM2_Copy(prev, now);
    }

    iVG_ColorSet(color);
//    SDL_RenderGeometry(renderer, NULL, verts, ARRLEN(verts), NULL, 0);
}

void VG_FillCircle(float *pos, float r, float* color) {
    VG_FillPolygon(pos, r, 0, 32, color);
}


void VG_DrawLine(float* from, float* to, float* color) {
    iVG_ColorSet(color);
    //SDL_RenderLine(renderer,
//		   from[0], from[1],
//		   to[0], to[1]);
}

void VG_DrawLines(float* points, uint32_t amount, float* color) {
    iVG_ColorSet(color);
    for (int i = 0; i < amount - 2; i += 2) {
	//SDL_RenderLine(renderer, points[i], points[i+1], points[i+2], points[i+3]);
    }
}

void VG_ClearBackground() {
    VG_ClearScreen(background_color);
}

void iVG_ColorSet(float *color) {
    glColor4f(color[0], color[1], color[2], color[3]);
//    SDL_SetRenderDrawColorFloat(renderer, color[0], color[1], color[2], color[3]);
}

void VG_ClearScreen(float* color) {
    //iVG_ColorSet(color);
    VG_SetBackgroundColor(color);
    glClear(GL_COLOR_BUFFER_BIT);
    //SDL_RenderClear(renderer);    
}

void VG_RenderFlush() {
//    SDL_RenderPresent(renderer);
    SDL_GL_SwapWindow(window);
}

const Uint8 *VG_GetKeys(uint32_t *number_of_keys) {
    return SDL_GetKeyboardState(number_of_keys);
}

void iVG_KeysUpdate() {
    keys = VG_GetKeys(NULL);
}

SDL_FPoint iVG_FPoint2(float* point) {
    return (SDL_FPoint){ point[0], point[1] };
}

SDL_Color iVG_Color4(float* color) {
    return (SDL_Color){ color[0]*255, color[1]*255, color[2]*255, color[3]*255 };
}

SDL_Vertex iVG_Vertex(float* point, float* color, float* tex_coord) {
    SDL_Vertex vertex;
    vertex.position = iVG_FPoint2(point);
    vertex.color = iVG_Color4(color);
    vertex.tex_coord = iVG_FPoint2(tex_coord);
    return vertex;
}

SDL_Vertex iVG_VertexColored(float* point, float* color) {
    return iVG_Vertex(point, color, VM2_ZERO);
}

void iVG_SetupOpengl() {
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);

    glClearColor(0,0,0,0);
    glViewport(0, 0, window_size[0], window_size[1]);
}

void iVG_GLFillRect(float* pos, float* size, float* color) {
    iVG_ColorSet(color);
//    glRectf(pos[0], pos[1], pos[0]+size[0], pos[1]+size[1]);
}
