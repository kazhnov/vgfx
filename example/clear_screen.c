#define _VMATH_IMPLEMENTATION_
#define _VCOLOR_IMPLEMENTATION_
#include "../vgfx.h"

static float size[2] = {1280.f, 720.f};

int main() {
    VG_WindowOpen("Example: Clear Screen", size, 0);
    VG_BackgroundColorSet(VRGBA_GREEN);
    
    while (!VG_WindowShouldClose()) {
	VG_DrawingBegin();
	VG_DrawingEnd();
    }

    VG_WindowClose();
}
