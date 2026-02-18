#define _VMATH_IMPLEMENTATION_
#define _VCOLOR_IMPLEMENTATION_
#include "../include/vcolor/vcolor.h"
#include "../vgfx.h"
#include <string.h>
#include <stdio.h>
#include <math.h>

static u32 model_bunny;
static u32 model_teapot;
static u32 model_floor;
static u32 shader_default;
static u32 shader_light;
static u32 texture_default;
static u32 flashlight;

static f32 size[2] = {720.f, 720.f};

typedef struct {
    f32 pos[3];
    f32 size[3];
    f32 rot[3];
    u32 model;
} Object;

static Object sun;
static u32 sunlight;

void GAME_HandleInput(Camera* camera) {
    f64 dt = VG_DeltaTimeGet();
    f32 forward[3];
    VG_CameraForwardGet(forward);
    f32 forward_scaled[3];
    VM3_ScaleO(forward, dt, forward_scaled);
    
    f32 right[3];
    VG_CameraRightGet(right);
    f32 right_scaled[3];
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
    if (VG_KeyDown(VG_KEY_SPACE)) {
	camera->position[1] += dt;
    }
    if (VG_KeyDown(VG_KEY_LEFT_SHIFT)) {
	camera->position[1] -= dt;
    }
    
    f32 mouse_delta[2];
    f32 sensitivity = 0.003;
    VG_MouseDeltaGet(mouse_delta);
    camera->rotation[1] -= mouse_delta[0]*sensitivity;
//    camera->rotation[0] -= mouse_delta[1]*sensitivity;
}

void GAME_BunniesInit(Object bunnies[], u32 count) {
    for (u32 i = 0; i < count; i++) {
	bunnies[i].model = model_bunny;
	VM3_Set(bunnies[i].size, 0.5f, 0.5f, 0.5f);
	VM3_Set(bunnies[i].pos,  i%(u32)sqrt(count), -0.5f, i/(u32)sqrt(count)  );
	VM3_Set(bunnies[i].rot, 0, 0, 0);
    }
}

void GAME_BunniesDraw(Object bunnies[], u32 count) {
    for (u32 i = 0; i < count; i++) {
	VG_ModelDrawAt(bunnies[i].model, bunnies[i].pos, bunnies[i].rot, bunnies[i].size);
    }
}


void GAME_LightUpdate(Camera* camera) {
    PointLight* light = VG_PointLightGet(sunlight);
    f64 time = VG_TimeGet();
    f32 pos[3];
    pos[2] = 0;
    pos[1] = fabs(cosf(time));
    pos[0] = fabs(sinf(time));
    VM3_Copy(sun.pos, pos);
    VM3_Copy(light->position, pos);
    VM3_Copy(light->color, pos);
    VG_ModelColorSet(sun.model, pos);
    

    FlashLight* flash_light = VG_FlashLightGet(flashlight);
    VM3_Copy(flash_light->position, camera->position);
    f32 direction[3] = {0, 0, -1};
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
//    VG_FPSMaxSet(60);
    shader_light = VG_ShaderLoad("shaders/shader.vert", "shaders/light.frag");
    shader_default = VG_ShaderLoad("shaders/shader.vert", "shaders/shader.frag");

    texture_default = VG_TextureNew("include/vtex/textures/default.ppm");
    u32 texture_bunny = VG_TextureNew("include/vtex/textures/input.ppm");
    
    VG_TextureDefaultSet(texture_default);
    model_teapot = VG_ModelNew("models/teapot.obj", 0, shader_light);
    model_bunny =  VG_ModelNew("models/bunny_textured.obj", texture_bunny, shader_default);
    model_floor =  VG_ModelNew("models/floor.obj", 0, shader_default);

    flashlight = VG_FlashLightCreate();
    
    sun.model = model_teapot;
    VM3_Set(sun.pos, 0.0, 0.0, 0.0);
    VM3_Set(sun.size, 0.1, 0.1, 0.1);
    VM3_Set(sun.rot, 5, 5, 5);

    u32 direct_handle = VG_DirectLightCreate();
    DirectLight* direct_light = VG_DirectLightGet(direct_handle);
    VM3_Copy(direct_light->color, VRGB_YELLOW);
    VM3_Set(direct_light->direction, 0, 1, 0);
    
    const u32 BUNNYC = 1000;
    Object bunnies[BUNNYC];
    GAME_BunniesInit(bunnies, BUNNYC);

    VG_BackgroundColorSet(VRGBA_BLACK);

    sunlight = VG_PointLightCreate();
    
    char fpsstring[1000];

    Camera* camera = VG_CameraGet();
    while (!VG_WindowShouldClose()) {
	GAME_LightUpdate(camera);

	snprintf(fpsstring, 100, "FPS: %d (%.6f)", (u32) VG_FPSGet(), VG_DeltaTimeGet());

	VG_WindowTitleSet(fpsstring);
	
	VG_DrawingBegin();
	GAME_HandleInput(camera);


	VG_ModelDrawAt(sun.model, sun.pos, sun.rot, sun.size);
	GAME_BunniesDraw(bunnies, BUNNYC);
	VG_ModelDrawAt(model_floor, (f32[]){0,-0.5,0}, (f32[]){0,0,0}, (f32[]){1, 1, 1});
	
	VG_DrawingEnd();
    }

    VG_WindowClose();
}
