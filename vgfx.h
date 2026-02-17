#include <stdbool.h>
#include <stdint.h>
#include "include/vmath/vmath.h"
#define VG_KEY_SPACE 32
#define VG_KEY_LEFT_SHIFT 340
#define VG_KEY_W 87
#define VG_KEY_S 83
#define VG_KEY_A 65
#define VG_KEY_D 68

#define VG_WINDOW_FLAG_VSYNC (1)

// INITIALIZATION AND CLOSING
void VG_WindowOpen(char* name, f32* size, u32 flags);

b8 VG_WindowShouldClose();

void VG_WindowClose();

void VG_WindowTitleSet(char* new);

const char* VG_WindowTitleGet();

void VG_VSyncSet(b8);

b8 VG_VSyncGet();

void VG_FPSMaxSet(u32);

// BACKGROUND COLOR
void VG_BackgroundColorGet(f32* out);

void VG_BackgroundColorSet(f32* in);


// WINDOW SIZE
void VG_WindowSizeGet(f32* out);

void VG_WindowSizeUpdate();


// SHADERS

u32 VG_ShaderLoad(const char* vertex_path, const char* fragment_path);

void VG_ShaderUse(u32 shader);

// FPS
f64 VG_FPSGet();

f64 VG_DeltaTimeGet();

f64 VG_TimeGet();


typedef struct {
    f32 position[3];
    f32 rotation[3];
    f32 fov;
} Camera;

// Camera
void VG_CameraPositionSet(f32 *pos);
void VG_CameraPositionGet(f32 *pos);
void VG_CameraRotationSet(f32 *pos);
void VG_CameraRotationGet(f32 *pos);
Camera *VG_CameraGet();
void VG_CameraSet(Camera camera);
void VG_CameraForwardGet(f32* out);
void VG_CameraRightGet(f32* out);

//LIGHT
typedef struct {
    f32 position[3];
    f32 color[3];
} PointLight;

typedef struct {
    f32 direction[3];
    f32 color[3];
} DirectLight;

typedef struct {
    f32 direction[3];
    f32 position[3];
    f32 color[3];
    f32 angle;
    f32 cutoff;
} FlashLight;

u32 VG_FlashLightCreate();
u32 VG_DirectLightCreate();
u32 VG_PointLightCreate();
u32 VG_FlashLightDestroy();
u32 VG_DirectLightDestroy();
u32 VG_PointLightDestroy();

FlashLight*  VG_FlashLightGet(u32 light);
DirectLight* VG_DirectLightGet(u32 light);
PointLight*  VG_PointLightGet(u32 light);


// TEXTURE
u32 VG_TextureNew(char* path);

void VG_TextureDefaultSet(u32 texture_handle);


// DRAWING MODES
void VG_DrawingBegin();

void VG_DrawingEnd();

// CLEARING SCREEN
void VG_Clear(f32* color);

void VG_BackgroundClear();


// KEYS
b8 VG_KeyPressed(u64 key);

b8 VG_KeyDown(u64 key);

const u8 *VG_KeysGet(u32 *number_of_keys);


// MOUSE
void VG_MouseDeltaGet(f32* out);

void VG_MouseGet(f32* out);

// MESHES
u32 VG_ModelNew(char* path, u32 texture, u32 shader);
void VG_ModelInstancesDraw(u32 model_handle);
void VG_ModelInstancesClear(u32 model_handle);
void     VG_ModelDrawAt(u32 model_handle, f32 pos[static 3], f32 rotation[static 3], f32 size[static 3]);
void     VG_ModelColorSet(u32 model_handle, f32 color[static 3]);

// DRAWING SHAPES

void VG_FillRect(f32* pos, f32* size, f32* color);

void VG_FillRectCentered(f32* pos, f32* size, f32* color);

void VG_DrawCircle(f32* pos, f32 r, f32* color);

void VG_FillPolygon(f32 *pos, f32 r, f32 angle, u32 sides, f32* color);

void VG_FillCircle(f32 *pos, f32 r, f32* color);

void VG_DrawLine(f32* from, f32* to, f32* color);

void VG_DrawLines(f32* points, u32 amount, f32* color);


