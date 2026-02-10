#include "include/vmath/vmath.h"
#include "include/vcolor/vcolor.h"
#include <stdbool.h>
#include <stdint.h>
#define VG_KEY_SPACE 32
#define VG_KEY_LEFT_SHIFT 340
#define VG_KEY_W 87
#define VG_KEY_S 83
#define VG_KEY_A 65
#define VG_KEY_D 68

#define VG_WINDOW_FLAG_VSYNC (1)

// INITIALIZATION AND CLOSING
void VG_WindowOpen(char* name, float* size, uint32_t flags);

bool VG_WindowShouldClose();

void VG_WindowClose();

void VG_WindowTitleSet(char* new);

const char* VG_WindowTitleGet();

void VG_VSyncSet(bool);

bool VG_VSyncGet();


// BACKGROUND COLOR
void VG_BackgroundColorGet(float* out);

void VG_BackgroundColorSet(float* in);


// WINDOW SIZE
void VG_WindowSizeGet(float* out);

void VG_WindowSizeUpdate();


// SHADERS

uint32_t VG_ShaderLoad(const char* vertex_path, const char* fragment_path);

void VG_ShaderUse(uint32_t shader);

// FPS
double VG_FPSGet();

double VG_DeltaTimeGet();

double VG_TimeGet();


typedef struct {
    float position[3];
    float rotation[3];
    float fov;
} Camera;

// Camera
void VG_CameraPositionSet(float *pos);
void VG_CameraPositionGet(float *pos);
void VG_CameraRotationSet(float *pos);
void VG_CameraRotationGet(float *pos);
Camera *VG_CameraGet();
void VG_CameraSet(Camera camera);
void VG_CameraForwardGet(float* out);
void VG_CameraRightGet(float* out);

//LIGHT
typedef enum LightType: uint8_t {
    LIGHT_TYPE_DIRECTIONAL,
    LIGHT_TYPE_POINT,
	
    LIGHT_TYPE_COUNT,
} LightType;

typedef struct {
    float direction[3];
    float color[3];
    LightType type;
} Light;

Light *VG_LightGet();

void VG_LightDirectionSet(float* dir);
void VG_LightDirectionGet(float* out);

void VG_LightPositionSet(float* dir);
void VG_LightPositionGet(float* out);

void VG_LightColorSet(float* color);
void VG_LightColorGet(float* out);

void VG_LightAmbientColorSet(float* color);
void VG_LightAmbientColorGet(float* out);

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


// MOUSE
void VG_MouseDeltaGet(float* out);

void VG_MouseGet(float* out);

// MESHES
uint32_t VG_ModelNew(char* path, uint32_t shader);
void     VG_ModelDrawAt(uint32_t model_handle, float pos[static 3], float size[static 3]);

// DRAWING SHAPES

void VG_FillRect(float* pos, float* size, float* color);

void VG_FillRectCentered(float* pos, float* size, float* color);

void VG_DrawCircle(float* pos, float r, float* color);

void VG_FillPolygon(float *pos, float r, float angle, uint32_t sides, float* color);

void VG_FillCircle(float *pos, float r, float* color);

void VG_DrawLine(float* from, float* to, float* color);

void VG_DrawLines(float* points, uint32_t amount, float* color);


