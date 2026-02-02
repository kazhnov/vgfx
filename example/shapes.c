#define _VMATH_IMPLEMENTATION_
#define _VCOLOR_IMPLEMENTATION_
#include "../vgfx.h"

static float size[2] = {720.f, 720.f};

int main() {
    VG_WindowOpen("Example: Clear Screen", size, 0);
    VG_BackgroundColorSet(VRGBA_BLUE);
    float center[2];
    
    while (!VG_WindowShouldClose()) {
	VG_DrawingBegin();
	VG_WindowSizeGet(size);
	VM2_Set(center, 0.4, 0.4);
	
	VG_FillRect(center, (float[]){.2f, .2f}, VRGBA_YELLOW);
	VG_DrawingEnd();
    }

    VG_WindowClose();
}
