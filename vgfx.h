#include "include/vmath/vmath.h"
#include <stdbool.h>
#include <stdint.h>

#define VG_WINDOW_FLAG_VSYNC (1)

void VG_InitWindow(char* name, float* size, uint32_t flags);

void VG_SetFlags(uint32_t flags);

bool VG_WindowShouldClose();

void VG_RenderFlush();


void VG_GetWindowSize(float* out);

void VG_UpdateWindowSize();


bool VG_IsKeyPressed(uint64_t key);

bool VG_IsKeyDown(uint64_t key);

uint64_t *VG_GetKeys();


void VG_DrawRect(float* pos, float* size, float* color);

void VG_FillRect(float* pos, float* size, float* color);

void VG_DrawCircle(float* pos, float r, float* color);

void VG_FillCircle(float* pos, float r, float* color);

void VG_ClearBackground(float* color);


