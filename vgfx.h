#include "include/vmath/vmath.h"
#include "include/vcolor/vcolor.h"
#include <stdbool.h>
#include <stdint.h>

#define VG_WINDOW_FLAG_VSYNC (1)

void VG_WindowOpen(char* name, float* size, uint32_t flags);

void VG_FlagsSet(uint32_t flags);

bool VG_WindowShouldClose();

void VG_WindowClose();


void VG_BackgroundColorGet(float* out);

void VG_BackgroundColorSet(float* in);


void VG_WindowSizeGet(float* out);


void VG_DrawingBegin();

void VG_DrawingEnd();

void VG_WindowSizeUpdate();


bool VG_KeyPressed(uint64_t key);

bool VG_KeyDown(uint64_t key);

const uint8_t *VG_KeysGet(uint32_t *number_of_keys);


void VG_Clear(float* color);

void VG_BackgroundClear();
