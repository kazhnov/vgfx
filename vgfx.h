#include "include/vmath/vmath.h"
#include "include/vcolor/vcolor.h"
#include <stdbool.h>
#include <stdint.h>

#define VG_WINDOW_FLAG_VSYNC (1)

void VG_InitWindow(char* name, float* size, uint32_t flags);

void VG_SetFlags(uint32_t flags);

bool VG_WindowShouldClose();

void VG_RenderFlush();


void VG_GetBackgroundColor(float* out);

void VG_SetBackgroundColor(float* in);


void VG_GetWindowSize(float* out);


void VG_DrawingBegin();

void VG_DrawingEnd();

void VG_UpdateWindowSize();


bool VG_IsKeyPressed(uint64_t key);

bool VG_IsKeyDown(uint64_t key);

const bool *VG_GetKeys(uint32_t *number_of_keys);


void VG_DrawRect(float* pos, float* size, float* color);

void VG_FillPolygon(float *pos, float r, float angle, uint32_t sides, float* color);

void VG_FillRect(float* pos, float* size, float* color);

void VG_FillRectCentered(float* pos, float* size, float* color);

void VG_DrawCircle(float* pos, float r, float* color);

void VG_FillCircle(float* pos, float r, float* color);

void VG_ClearScreen(float* color);

void VG_ClearBackground();
