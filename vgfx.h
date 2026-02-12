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
typedef struct {
    float position[3];
    float color[3];
} PointLight;

typedef struct {
    float direction[3];
    float color[3];
} DirectLight;

typedef struct {
    float direction[3];
    float position[3];
    float color[3];
    float angle;
    float cutoff;
} FlashLight;


uint32_t VG_FlashLightCreate();
uint32_t VG_DirectLightCreate();
uint32_t VG_PointLightCreate();
uint32_t VG_FlashLightDestroy();
uint32_t VG_DirectLightDestroy();
uint32_t VG_PointLightDestroy();

FlashLight*  VG_FlashLightGet(uint32_t light);
DirectLight* VG_DirectLightGet(uint32_t light);
PointLight*  VG_PointLightGet(uint32_t light);

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
void     VG_ModelDrawAt(uint32_t model_handle, float pos[static 3], float rotation[static 3], float size[static 3]);
void     VG_ModelColorSet(uint32_t model_handle, float color[static 3]);

// DRAWING SHAPES

void VG_FillRect(float* pos, float* size, float* color);

void VG_FillRectCentered(float* pos, float* size, float* color);

void VG_DrawCircle(float* pos, float r, float* color);

void VG_FillPolygon(float *pos, float r, float angle, uint32_t sides, float* color);

void VG_FillCircle(float *pos, float r, float* color);

void VG_DrawLine(float* from, float* to, float* color);

void VG_DrawLines(float* points, uint32_t amount, float* color);


