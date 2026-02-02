#define _VMATH_IMPLEMENTATION_
#define _VCOLOR_IMPLEMENTATION_
#include "../vgfx.h"
#include <string.h>
#include <stdio.h>

static float size[2] = {720.f, 720.f};

int main() {
    VG_WindowOpen("Example: Clear Screen", size, 0);
    VG_BackgroundColorSet(VRGBA_BLUE);
    float center[2] = {0, 0};
    float radius = 0.1;
    float angle  = 0.;

    char buffer[256];

//    VG_VSyncSet(0);
    while (!VG_WindowShouldClose()) {
	angle += VG_DeltaTimeGet();
	snprintf(buffer, 255, "FPS: %.0f", VG_FPSGet());
	printf("FPS: %.0f\n", VG_FPSGet());
	VG_WindowTitleSet(buffer);
	float offset[] = {radius, 0};
	VM2_Rotate(offset, angle);
	float pos[2];
	VM2_AddO(center, offset, pos);
	
	VG_DrawingBegin();
	
	VG_FillRectCentered(pos, (float[]){.2f, .2f}, VRGBA_YELLOW);
	
	VG_DrawingEnd();
    }

    VG_WindowClose();
}
