#define _VMATH_IMPLEMENTATION_
#define _VCOLOR_IMPLEMENTATION_
#include "../vgfx.h"
#include <string.h>
#include <stdio.h>

static Mesh* mesh_bunny;
static Mesh* mesh_teapot;
static float size[2] = {720.f, 720.f};
static Entity* sun;


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

    float mouse_delta[2];
    float sensitivity = 0.003;
    VG_MouseDeltaGet(mouse_delta);
    camera->rotation[1] -= mouse_delta[0]*sensitivity;
//    camera->rotation[0] -= mouse_delta[1]*sensitivity;
}

void GAME_BunniesInit(Entity* bunnies[], uint32_t count) {
    for (int i = 0; i < count; i++) {
	bunnies[i] = VG_EntityCreate();
	printf("b %d\n", VMESH_VertexCount(mesh_bunny));
	VG_EntityMeshSet(bunnies[i], mesh_bunny);
	printf("c\n");
	
	VG_EntityPosSet(bunnies[i], (float[]){0, 0., i});
	VG_EntitySizeSet(bunnies[i], 0.5f);
    }
}

void GAME_BunniesDraw(Entity* bunnies[], uint32_t count) {
    for (int i = 0; i < count; i++) {
	VG_EntityDraw(bunnies[i]);
    }
}


void GAME_LightUpdate() {
    double time = VG_TimeGet();
    float pos[3];
    pos[1] = fabs(cosf(time));
    pos[0] = fabs(sinf(time));
    VG_LightPositionSet(pos);
    VG_LightColorSet(pos);
    VG_EntityPosSet(sun, pos);
}

int main() {
    VG_WindowOpen("Example: Clear Screen", size, 0);
    VG_BackgroundColorSet(VRGBA_BLACK);
    VG_VSyncSet(true);

    mesh_bunny = VMESH_LoadObj("/home/jkasinowe/projects/c/vmesh/bunny.obj");
    mesh_teapot = VMESH_LoadObj("/home/jkasinowe/projects/c/vgfx/teapot.obj");

    sun = VG_EntityCreate();
    VG_EntityMeshSet(sun, mesh_teapot);
    VG_EntityPosSet(sun, (float[]){0.0, 0.0, 0.0});
    VG_EntitySizeSet(sun, 0.1);
    
    printf("a\n");
    
    Entity* bunnies[2];
    GAME_BunniesInit(bunnies, 2);

    float background[3] = {0.0, 0.1, 0.3};
    VG_BackgroundColorSet(background);
    Camera* camera = VG_CameraGet();
    VG_LightPositionSet((float[]){0.5, 0, 0});
    VG_LightColorSet((float[]){1.0, 1.0, 0.0});
    VG_LightAmbientColorSet(background);
    
    while (!VG_WindowShouldClose()) {
	GAME_LightUpdate();
	
	VG_DrawingBegin();
	GAME_HandleInput(camera);

	GAME_BunniesDraw(bunnies, 2);
	VG_EntityDraw(sun);
	
	VG_DrawingEnd();
    }

    VG_WindowClose();
}
