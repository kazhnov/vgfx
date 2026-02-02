#include "include/vmath/vmath.h"
#include "include/vcolor/vcolor.h"
#include <stdbool.h>
#include <stdint.h>

#define VG_WINDOW_FLAG_VSYNC (1)

// INITIALIZATION AND CLOSING
void VG_WindowOpen(char* name, float* size, uint32_t flags);

bool VG_WindowShouldClose();

void VG_WindowClose();

void VG_WindowTitleSet(char* new);

void VG_WindowTitleGet(char* out);

void VG_VSyncSet(bool);

bool VG_VSyncGet();

// BACKGROUND COLOR
void VG_BackgroundColorGet(float* out);

void VG_BackgroundColorSet(float* in);


// WINDOW SIZE
void VG_WindowSizeGet(float* out);

void VG_WindowSizeUpdate();


// FPS
double VG_FPSGet();

double VG_DeltaTimeGet();


// DRAWING MODES
void VG_DrawingBegin();

void VG_DrawingEnd();

// CLEARING SCREEN
void VG_Clear(float* color);

void VG_BackgroundClear();


// KEYS
bool VG_KeyPressed(uint64_t key);

bool VG_KeyDown(uint64_t key);

const uint8_t *VG_KeysGet(uint32_t *number_of_keys);

// DRAWING SHAPES

void VG_FillRect(float* pos, float* size, float* color);

void VG_FillRectCentered(float* pos, float* size, float* color);

void VG_DrawCircle(float* pos, float r, float* color);

void VG_FillPolygon(float *pos, float r, float angle, uint32_t sides, float* color);

void VG_FillCircle(float *pos, float r, float* color);

void VG_DrawLine(float* from, float* to, float* color);

void VG_DrawLines(float* points, uint32_t amount, float* color);


