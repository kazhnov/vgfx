#define _VMATH_IMPLEMENTATION_
#define _VCOLOR_IMPLEMENTATION_
#include "../include/vcolor/vcolor.h"
#include "../vgfx.h"
#include <string.h>
#include <stdio.h>

static uint32_t model_bunny;
static uint32_t model_teapot;
static uint32_t shader_default;
static uint32_t shader_light;
static uint32_t flashlight;

static float size[2] = {720.f, 720.f};

typedef struct {
    float pos[3];
    float size[3];
    float rot[3];
    uint32_t model;
} Object;

static Object sun;
static uint32_t sunlight;

void GAME_HandleInput(Camera* camera) {
    double dt = VG_DeltaTimeGet();
    float forward[3];
    VG_CameraForwardGet(forward);
    float forward_scaled[3];
    VM3_ScaleO(forward, dt, forward_scaled);
    
    float right[3];
    VG_CameraRightGet(right);
    float right_scaled[3];
    VM3_ScaleO(right, dt, right_scaled);
    
    
    if (VG_KeyDown(VG_KEY_W)) {
	VM3_Add(camera->position, forward_scaled);
    }
    if (VG_KeyDown(VG_KEY_S)) {
	VM3_Subtract(camera->position, forward_scaled);
    }
    if (VG_KeyDown(VG_KEY_D)) {
	VM3_Add(camera->position, right_scaled);
    }
    if (VG_KeyDown(VG_KEY_A)) {
	VM3_Subtract(camera->position, right_scaled);
    }
    if (VG_KeyPressed(VG_KEY_SPACE)) {
	printf("jump!\n");
    }
    
    float mouse_delta[2];
    float sensitivity = 0.003;
    VG_MouseDeltaGet(mouse_delta);
    camera->rotation[1] -= mouse_delta[0]*sensitivity;
//    camera->rotation[0] -= mouse_delta[1]*sensitivity;
}

void GAME_BunniesInit(Object bunnies[], uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
	bunnies[i].model = model_bunny;
	VM3_Set(bunnies[i].size, 0.5f, 0.5f, 0.5f);
	VM3_Set(bunnies[i].pos,  i%10, 0.f, i/10  );
    }
}

void GAME_BunniesDraw(Object bunnies[], uint32_t count) {
    for (uint32_t i = 0; i < count; i++) {
	VG_ModelDrawAt(bunnies[i].model, bunnies[i].pos, bunnies[i].rot, bunnies[i].size);
    }
}


void GAME_LightUpdate(Camera* camera) {
    PointLight* light = VG_PointLightGet(sunlight);
    double time = VG_TimeGet();
    float pos[3];
    pos[2] = 0;
    pos[1] = fabs(cosf(time));
    pos[0] = fabs(sinf(time));
    VM3_Copy(sun.pos, pos);
    VM3_Copy(light->position, pos);
    VM3_Copy(light->color, pos);
    VG_ModelColorSet(sun.model, pos);
    

    FlashLight* flash_light = VG_FlashLightGet(flashlight);
    VM3_Copy(flash_light->position, camera->position);
    float direction[3] = {0, 0, -1};
    VM3_RotateY(direction, camera->rotation[1]);
    VM3_Copy(flash_light->direction, direction);
    VM3_Copy(flash_light->color, VRGB_BLUE);
    flash_light->angle = V_PI/12;
    flash_light->cutoff = V_PI/12;
}

int main() {
    VG_WindowOpen("Example: Meshes", size, 0);
    VG_BackgroundColorSet(VRGBA_BLACK);
    VG_VSyncSet(false);

    shader_light = VG_ShaderLoad("shaders/shader.vert", "shaders/light.frag");
    shader_default = VG_ShaderLoad("shaders/shader.vert", "shaders/shader.frag");
    
    model_teapot = VG_ModelNew("models/teapot.obj", shader_light);
    model_bunny = VG_ModelNew("include/vmesh/bunny_flatobj.obj", shader_default);

    flashlight = VG_FlashLightCreate();
    
    sun.model = model_teapot;
    VM3_Set(sun.pos, 0.0, 0.0, 0.0);
    VM3_Set(sun.size, 0.1, 0.1, 0.1);
    VM3_Set(sun.rot, 5, 5, 5);

    uint32_t direct_handle = VG_DirectLightCreate();
    DirectLight* direct_light = VG_DirectLightGet(direct_handle);
    VM3_Copy(direct_light->color, VRGB_YELLOW);
    VM3_Set(direct_light->direction, 0, 1, 0);
    
    const uint32_t BUNNYC = 100;
    Object bunnies[BUNNYC];
    GAME_BunniesInit(bunnies, BUNNYC);

    VG_BackgroundColorSet(VRGBA_BLACK);

    sunlight = VG_PointLightCreate();
    
    char fpsstring[100];

    Camera* camera = VG_CameraGet();
    while (!VG_WindowShouldClose()) {
	GAME_LightUpdate(camera);

	snprintf(fpsstring, 100, "FPS: %d (%.6f)", (uint32_t) VG_FPSGet(), VG_DeltaTimeGet());

	VG_WindowTitleSet(fpsstring);
	
	VG_DrawingBegin();
	GAME_HandleInput(camera);

	GAME_BunniesDraw(bunnies, BUNNYC);
	
	VG_ModelDrawAt(sun.model, sun.pos, sun.rot, sun.size);
	
	VG_DrawingEnd();
    }

    VG_WindowClose();
}
