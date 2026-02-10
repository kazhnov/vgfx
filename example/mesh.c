#define _VMATH_IMPLEMENTATION_
#define _VCOLOR_IMPLEMENTATION_
#include "../vgfx.h"
#include <string.h>
#include <stdio.h>

static uint32_t model_bunny;
static uint32_t model_teapot;
static uint32_t shader_default;

static float size[2] = {720.f, 720.f};

typedef struct {
    float pos[3];
    float size[3];
    uint32_t model;
} Object;

static Object sun;

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
	VG_ModelDrawAt(bunnies[i].model, bunnies[i].pos, bunnies[i].size);
    }
}


void GAME_LightUpdate() {
    double time = VG_TimeGet();
    float pos[3];
    pos[1] = fabs(cosf(time));
    pos[0] = fabs(sinf(time));
    VG_LightPositionSet(pos);
    VG_LightColorSet(pos);
    VM3_Copy(sun.pos, pos);
}

int main() {
    VG_WindowOpen("Example: Meshes", size, 0);
    VG_BackgroundColorSet(VRGBA_BLACK);
    VG_VSyncSet(true);

    shader_default = VG_ShaderLoad("shaders/shader.vert", "shaders/shader.frag");
    VG_ShaderUse(shader_default);
    
    model_bunny = VG_ModelNew("include/vmesh/bunny_flatobj.obj", 0);
    model_teapot = VG_ModelNew("models/teapot.obj", 0);

    sun.model = model_teapot;
    VM3_Set(sun.pos, 0.0, 0.0, 0.0);
    VM3_Set(sun.size, 0.1, 0.1, 0.1);
    
    const uint32_t BUNNYC = 100;
    Object bunnies[BUNNYC];
    GAME_BunniesInit(bunnies, BUNNYC);

    float background[4] = {0.1, 0.1, 0., 1.0};
    VG_BackgroundColorSet(background);
    Camera* camera = VG_CameraGet();
    VG_LightPositionSet((float[]){0.5, 0, 0});
    VG_LightColorSet((float[]){1.0, 1.0, 0.0});
    VG_LightAmbientColorSet(background);

    char fpsstring[100];
    
    while (!VG_WindowShouldClose()) {
	GAME_LightUpdate();

	snprintf(fpsstring, 100, "FPS: %d (%.6f)", (uint32_t) VG_FPSGet(), VG_DeltaTimeGet());

	VG_WindowTitleSet(fpsstring);
	
	VG_DrawingBegin();
	GAME_HandleInput(camera);

	GAME_BunniesDraw(bunnies, BUNNYC);
	
	VG_ModelDrawAt(sun.model, sun.pos, sun.size);
	
	VG_DrawingEnd();
    }

    VG_WindowClose();
}
