#include "vgfx.h"
#include "include/vcolor/vcolor.h"
#include "include/vtex/vtex.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "include/vmesh/vmesh.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#define ARRLEN(x) ((sizeof(x))/(sizeof(x[0])))

#if 0
#define iVG_Log(a) printf(a "\n")
#else
#define iVG_Log(a)
#endif

static GLFWwindow *window = NULL;
static f32 background_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
static f32 window_size[2];
static f64 current_time;
static f64 previous_time;
static b8 vsync;
static u32 shader_current;
static u32 texture_default;
static u32 texture_current;

static Camera camera;

static f32 matrix_view[16];
static f32 matrix_projection[16];

static b8  mouse_first = true;
static f32 mouse_last[2];
static f32 mouse_delta[2];

static b8 keys_just_pressed[512];
static b8 keys_pressed[512];
void iVG_KeysJustPressedClear();

const u32 dims = 3;
const u32 normals = 3;
const u32 uvs = 2;
const u32 vert_size = dims + normals + uvs;


#define POINT_LIGHT_MAX_COUNT  8
#define DIRECT_LIGHT_MAX_COUNT 8
#define FLASH_LIGHT_MAX_COUNT  2
PointLight  pointLights[POINT_LIGHT_MAX_COUNT];
DirectLight directLights[DIRECT_LIGHT_MAX_COUNT];
FlashLight  flashLights[FLASH_LIGHT_MAX_COUNT];
u32 pointLightCount = 0;
u32 directLightCount = 0;
u32 flashLightCount = 0;

void iVG_RenderFlush();
void iVG_KeysUpdate();
void iVG_SetupOpengl();
void iVG_FramebufferSizeCallback(GLFWwindow *window, int width, int height);
void iVG_MouseCallback(GLFWwindow* window, f64 x, f64 y);
void iVG_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void iVG_InputUpdate();
void iVG_PollEvents();

char* iVG_FileLoadToString(const char* path);

typedef struct {
    u32 VAO;
    u32 index_count;
    u32 shader;
    f32 color[3];
    u32 texture;
} Model;

// MODELARENA
typedef struct {
    Model* base;
    u32 position;
    u32 size;
} ModelArena;

static ModelArena model_arena;

void     iVG_ModelArenaInit(u32 size);
u32 iVG_ModelArenaBump();
Model*   iVG_ModelArenaPointerGet(u32 model_handle);
void     iVG_ModelArenaDestroy();

typedef struct {
    u32* base;
    u32 position;
    u32 size;
} TextureArena;

static TextureArena texture_arena;

void      iVG_TextureArenaInit(u32 size);
u32  iVG_TextureArenaBump();
u32* iVG_TextureArenaPointerGet(u32 model_handle);
void      iVG_TextureArenaDestroy();
void iVG_TextureUse(u32 texture);


// BUFFERING DATA
typedef u32 VAO_t;
typedef u32 VBO_t;
VAO_t iVG_GLVertexArrayNew();
void  iVG_GLVertexArrayBind(VAO_t VAO);
void  iVG_GLBufferData(u32 pointer, f32* vertices, u32 arr_size, u32 flags, u32 stride);
void  iVG_GLDrawTriangles(VAO_t amount);
void  iVG_GLVertexArrayDestroy(VAO_t VAO);
void  iVG_GLModelRender(Model *VAO);
u32 iVG_GLLoadVerticesIndexed(Vertex* vertices, u32 vcount, u32* indices, u32 icount);
void  iVG_GLRenderVerticesIndexed(Vertex* vertices, u32 vcound, u32 *indices, u32 icount);
void  iVG_GLTransformSet(f32* matrix);


void iVG_LightInit();

void iVG_GLCameraUpdate();
void iVG_GLPerspectiveUpdate();
void iVG_GLLightUpdate();
void iVG_GLShaderCameraUpdate();
void iVG_GLShaderLightUpdate();
void iVG_GLShaderProjectionUpdate();


void iVG_GLUniformVec3Set(char* name, f32* vec);
void iVG_GLUniformF32Set(char* name, f32 value);
void iVG_GLUniformIntSet(char *name, int value);


// INITIALIZATION AND CLOSING
void VG_WindowOpen(char* name, f32* size, u32 flags) {
    VM2_Copy(window_size, size);

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    window = glfwCreateWindow(size[0], size[1], name, NULL, NULL);
    if (!window) {
	printf("Failed to create GLFW window\n");
	glfwTerminate();
	exit(1);
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, iVG_FramebufferSizeCallback);
    glfwSetCursorPosCallback(window, iVG_MouseCallback);
    glfwSetKeyCallback(window, iVG_KeyCallback);
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (!gladLoadGLLoader((GLADloadproc)(glfwGetProcAddress))) {
	printf("Failed to initiate GLAD\n");
	glfwTerminate();
	exit(1);
    }

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    
    VG_VSyncSet(0);
    
    current_time = glfwGetTime();

    camera.fov = V_PI/2;
    
    f32 mat[16] = VM44_IDENTITY;
    iVG_GLTransformSet(mat);
    iVG_ModelArenaInit(64);
    iVG_TextureArenaInit(64);
    iVG_LightInit();
}

b8 VG_WindowShouldClose() {
    return glfwWindowShouldClose(window);
}

void VG_WindowClose() {
    iVG_ModelArenaDestroy();
    glfwTerminate();
}

void VG_WindowTitleSet(char* new) {
    glfwSetWindowTitle(window, new);
}

const char* VG_WindowTitleGet() {
    return glfwGetWindowTitle(window);
}

void VG_VSyncSet(b8 value) {
    vsync = value;
    glfwSwapInterval(value);
}

b8 VG_VSyncGet() {
    return vsync;
}

// BACKGROUND COLOR
void VG_BackgroundColorSet(f32* in) {
    VRGBA_Copy(background_color, in);
}

void VG_BackgroundColorGet(f32* out) {
    VRGBA_Copy(out, background_color);
}


// WINDOW SIZE
void VG_WindowSizeGet(f32* out) {
    VM2_Copy(out, window_size);
}

void VG_WindowSizeUpdate() {
    VG_WindowSizeGet(window_size);
    glViewport(0, 0, window_size[0], window_size[1]);
}


// FPS
f64 VG_FPSGet() {
    return 1./VG_DeltaTimeGet();
}

f64 VG_DeltaTimeGet() {
    return current_time - previous_time;
}

f64 VG_TimeGet() {
    return current_time;
}

// CAMERA
void VG_CameraPositionSet(f32 *pos) {
    VM3_Copy(camera.position, pos);
}

void VG_CameraPositionGet(f32 *pos) {
    VM3_Copy(pos, camera.position);
}

void VG_CameraRotationSet(f32 *rot) {
    VM3_Copy(camera.rotation, rot);
}

void VG_CameraRotationGet(f32 *rot) {
    VM3_Copy(rot, camera.rotation);
}

void VG_CameraForwardGet(f32* out) {
    VM3_Set(out, 0, 0, -1);
    VM3_RotateY(out, camera.rotation[1]);
}

void VG_CameraRightGet(f32* out) {
    VM3_Set(out, 1, 0, 0);
    VM3_RotateY(out, camera.rotation[1]);
}

Camera *VG_CameraGet() {
    return &camera;
}

void VG_CameraSet(Camera camera) {
    VG_CameraPositionSet(camera.position);
    VG_CameraRotationSet(camera.rotation);
}


//LIGHT
u32 VG_FlashLightCreate() {
    if (flashLightCount >= FLASH_LIGHT_MAX_COUNT - 1) {
	fprintf(stderr, "Too many flashlights");
	exit(1);
    }
    return flashLightCount++;
}
u32 VG_DirectLightCreate() {
    if (directLightCount >= DIRECT_LIGHT_MAX_COUNT - 1) {
	fprintf(stderr, "Too many direct lights");
	exit(1);
    }
    return directLightCount++;
}
u32 VG_PointLightCreate() {
    if (pointLightCount >= POINT_LIGHT_MAX_COUNT - 1) {
	fprintf(stderr, "Too many point lights");
	exit(1);
    }
    return pointLightCount++;
}

FlashLight*  VG_FlashLightGet(u32 light) {
    return flashLights + light;
}

DirectLight* VG_DirectLightGet(u32 light) {
    return directLights + light;
}

PointLight*  VG_PointLightGet(u32 light) {
    return pointLights + light;
}

void iVG_LightInit() {
    memset(directLights, 0, sizeof(DirectLight)*DIRECT_LIGHT_MAX_COUNT);
    memset(pointLights,  0, sizeof(PointLight)*POINT_LIGHT_MAX_COUNT);
    memset(flashLights,  0, sizeof(FlashLight)*FLASH_LIGHT_MAX_COUNT);
}


// TEXTURE
u32 VG_TextureNew(char* path) {
    u32 texture_handle = iVG_TextureArenaBump();
    u32* texture = iVG_TextureArenaPointerGet(texture_handle);
    Texture texture_data = VTEX_LoadPPM(path);

    u32 texture_gl;
    glGenTextures(1, &texture_gl);
    glBindTexture(GL_TEXTURE_2D, texture_gl);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_data.width, texture_data.height,
		 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data.data);
    glBindTexture(GL_TEXTURE_2D, 0);
    *texture = texture_gl;
    
    return texture_handle;
}

void VG_TextureDefaultSet(u32 tex) {
    texture_default = tex;
}

// DRAWING MODES
void VG_DrawingBegin() {
    iVG_KeysJustPressedClear();
    iVG_InputUpdate();
    iVG_GLCameraUpdate();
    iVG_GLPerspectiveUpdate();
    VG_Clear(background_color);
    previous_time = current_time;
    current_time = glfwGetTime();
}

void VG_DrawingEnd() {
    iVG_RenderFlush();
    
}


// CLEARING SCREEN
void VG_Clear(f32* color) {
    glClearColor(color[0], color[1], color[2], color[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
}

void VG_BackgroundClear() {
    VG_Clear(background_color);
}

// KEYS
b8 VG_KeyPressed(u64 key) {
    return keys_just_pressed[key];
}

b8 VG_KeyDown(u64 key) {
    return glfwGetKey(window, key) == GLFW_PRESS;
}


//MOUSE
void VG_MouseDeltaGet(f32* out) {
    VM2_Copy(out, mouse_delta);
}

void VG_MouseGet(f32* out) {
    VM2_Copy(out, mouse_last);
}

// MODEL
u32 VG_ModelNew(char* path, u32 texture, u32 shader) {
    u32 model_handle = iVG_ModelArenaBump();
    Model* model = iVG_ModelArenaPointerGet(model_handle);
    Mesh* mesh = malloc(sizeof(Mesh));
    VMESH_LoadObj(mesh, path);
    model->VAO = iVG_GLLoadVerticesIndexed(mesh->vertices, mesh->vertex_count,
				     mesh->indices, mesh->index_count);
    model->index_count = mesh->index_count;
    model->shader = shader;
    model->texture = texture;
    VM3_Set(model->color, 1, 1, 1);
    return model_handle;
}

void VG_ModelDrawAt(u32 model_handle, f32 pos[static 3], f32 rotation[static 3], f32 size[static 3]) {
    Model* model = iVG_ModelArenaPointerGet(model_handle);
    f32 transform[16] = VM44_IDENTITY;
    VM44_Scale(transform, size);
    VM44_Rotate(transform, rotation);
    VM44_Translate(transform, pos);

    VG_ShaderUse(model->shader);
    
    if (model->texture) {
	iVG_TextureUse(model->texture);
    } else {
	iVG_TextureUse(texture_default);
    }
    
    iVG_GLTransformSet(transform);
    iVG_GLUniformVec3Set("material.color", model->color);
    
    iVG_GLModelRender(model);
}

void VG_ModelColorSet(u32 model_handle, f32 color[static 3]) {
    Model* model = iVG_ModelArenaPointerGet(model_handle);
    VM3_Copy(model->color, color);
}

// INTERNALS
void iVG_RenderFlush() {
    glfwSwapBuffers(window);
}

void iVG_SetupOpengl() {
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);

    glClearColor(0,0,0,0);
    glViewport(0, 0, window_size[0], window_size[1]);
}

void iVG_GLDrawTriangles(u32 amount) {
    glDrawArrays(GL_TRIANGLES, 0, 3*amount);
}

void iVG_GLVertexArrayBind(VAO_t VAO) {
    glBindVertexArray(VAO);
}

void iVG_GLVertexArrayUnbind() {
    glBindVertexArray(0);
}

VAO_t iVG_GLVertexArrayNew() {
    VAO_t VAO;
    glGenVertexArrays(1, &VAO);
    return VAO;
}

void iVG_GLVertexArrayDestroy(VAO_t VAO) {
    glDeleteVertexArrays(1, &VAO);
}


void iVG_GLBufferIndices(u32* indices, u32 arr_len, u32 flags) {
    u32 EBO;
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, arr_len*sizeof(u32), indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void iVG_GLBufferVertices(Vertex* vertices, u32 count) {
    u32 VBO;
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, count*sizeof(Vertex), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, pos)));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, normal)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, tex)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

u32 iVG_GLLoadVerticesIndexed(Vertex* vertices, u32 vcount, u32* indices, u32 icount) {
    u32 VAO = iVG_GLVertexArrayNew();
    iVG_GLVertexArrayBind(VAO);
    
    iVG_GLBufferVertices(vertices, vcount);
    
    iVG_GLBufferIndices(indices, icount, 0);
    iVG_GLVertexArrayUnbind();
    return VAO;
}

void iVG_GLModelRender(Model *model) {
    iVG_GLVertexArrayBind(model->VAO);
    
    glDrawElements(GL_TRIANGLES, model->index_count, GL_UNSIGNED_INT, NULL);

    iVG_GLVertexArrayBind(0);
}

char* iVG_FileLoadToString(const char* path) {
    FILE* file = fopen(path, "rb");
    if (file == NULL) {
	printf("ERROR: No file with path %s\n", path);
	exit(1);
    }

    fseek(file, 0, SEEK_END);
    u32 length = ftell(file);
    rewind(file);

    char* buffer = (char*)malloc(length + 1);

    size_t bytes_read = fread(buffer, 1, length, file);

    if (bytes_read != length) {
	fclose(file);
	free(buffer);
	printf("ERROR: Unable to read file\n");
	exit(1);
    }

    buffer[length] = '\0';

    fclose(file);
    
    return buffer;
}

// SHADERS
void VG_ShaderUse(u32 shader) {
    if (shader_current == shader) return;
    shader_current = shader;
    glUseProgram(shader);
    iVG_GLShaderCameraUpdate();
    iVG_GLShaderLightUpdate();
    iVG_GLShaderProjectionUpdate();
}

u32 VG_ShaderLoad(const char* vertex_path, const char* fragment_path) {
    u32 vertex_shader = glCreateShader(GL_VERTEX_SHADER);

    char* vertex_shader_source = iVG_FileLoadToString(vertex_path);
    glShaderSource(vertex_shader, 1, (const char**)&vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    free(vertex_shader_source);
    
    int32_t success;
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);

    if (!success) {
	char info_log[512];
	glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
	printf("VERTEX SHADER ERROR: %s\n", info_log);
	exit(1);
    }    

    u32 fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    char* fragment_shader_source = iVG_FileLoadToString(fragment_path);
    glShaderSource(fragment_shader, 1, (const char**)&fragment_shader_source, NULL);
    glCompileShader(fragment_shader);
    free(fragment_shader_source);
    
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);

    if (!success) {
	char info_log[512];
	glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
	printf("FRAGMENT SHADER ERROR: %s\n", info_log);
	exit(1);
    }

    int32_t shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);


    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if(!success) {
	char info_log[512];
	glGetProgramInfoLog(shader_program, 512, NULL, info_log);
	printf("ERROR: %s\n", info_log);
	exit(1);
    }
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return shader_program;
}

void iVG_FramebufferSizeCallback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
    VM2_Set(window_size, width, height);
}

void iVG_InputUpdate() {
    VM2_Set(mouse_delta, 0, 0);
    iVG_PollEvents();
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
	glfwSetWindowShouldClose(window, true);
    }
}

void iVG_PollEvents() {
    glfwPollEvents();
}

void iVG_VertexColoredGet(f32* point, f32* color, f32* out) {
    VM3_Copy(out, point);
    VRGBA_Copy(out+dims, color);
}

void  iVG_GLTransformSet(f32* matrix) {
    u32 transform_loc = glGetUniformLocation(shader_current, "model");
    glUniformMatrix4fv(transform_loc, 1, GL_TRUE, matrix);
}

void iVG_GLShaderProjectionUpdate() {
    u32 projection_loc = glGetUniformLocation(shader_current, "projection");
    glUniformMatrix4fv(projection_loc, 1, GL_TRUE, matrix_projection);
}

void iVG_GLPerspectiveUpdate() {
    f32 size[2];
    VG_WindowSizeGet(size);
    VM44_ProjectionPerspective(matrix_projection, camera.fov, size[0]/size[1], 0.1, 100);
}

void iVG_GLLightUpdate() {
    
}

void iVG_GLUniformVec3Set(char* name, f32* vec) {
    u32 loc = glGetUniformLocation(shader_current, name);
    glUniform3fv(loc, 1, vec);
}

void iVG_GLUniformIntSet(char *name, int i) {
    u32 loc = glGetUniformLocation(shader_current, name);
    glUniform1i(loc, i);
}


void iVG_GLUniformF32Set(char *name, f32 f) {
    u32 loc = glGetUniformLocation(shader_current, name);
    glUniform1f(loc, f);
}

void iVG_GLShaderCameraUpdate() {
    u32 view_loc = glGetUniformLocation(shader_current, "view");
    iVG_GLUniformVec3Set("cameraPos", camera.position);
    glUniformMatrix4fv(view_loc, 1, GL_TRUE, matrix_view);
}

void iVG_GLCameraUpdate() {
    f32 out[16];
    VM44_V3A3(camera.position, camera.rotation, matrix_view);
    VM44_InverseO(matrix_view, out);
    VM44_Copy(matrix_view, out);
}

void iVG_GLShaderLightUpdate() {
    char name[50];
    for(u32 i = 0; i < DIRECT_LIGHT_MAX_COUNT; i++) {
	snprintf(name, 49, "directLights[%d].direction", i);
	iVG_GLUniformVec3Set(name, directLights[i].direction);
	snprintf(name, 49, "directLights[%d].color", i);
	iVG_GLUniformVec3Set(name, directLights[i].color);
    }
    for (u32 i = 0; i < POINT_LIGHT_MAX_COUNT; i++) {
	snprintf(name, 49, "pointLights[%d].position", i);
	iVG_GLUniformVec3Set(name, pointLights[i].position);
	snprintf(name, 49, "pointLights[%d].color", i);
	iVG_GLUniformVec3Set(name, pointLights[i].color);
    }
    for (u32 i = 0; i < FLASH_LIGHT_MAX_COUNT; i++) {
	snprintf(name, 49, "flashLights[%d].position", i);
	iVG_GLUniformVec3Set(name, flashLights[i].position);
	snprintf(name, 49, "flashLights[%d].direction", i);
	iVG_GLUniformVec3Set(name, flashLights[i].direction);
	snprintf(name, 49, "flashLights[%d].color", i);
	iVG_GLUniformVec3Set(name, flashLights[i].color);
	snprintf(name, 49, "flashLights[%d].angle", i);
	iVG_GLUniformF32Set(name, flashLights[i].angle);
	snprintf(name, 49, "flashLights[%d].cutoff", i);
	iVG_GLUniformF32Set(name, flashLights[i].cutoff);
    }
}

void iVG_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
	keys_just_pressed[key] = true;
	keys_pressed[key] = true;
    } else if (action == GLFW_RELEASE) {
	keys_pressed[key] = false;
    }
}

void iVG_KeysJustPressedClear() {
    memset(keys_just_pressed, 0, 512*sizeof(b8));
}

void iVG_MouseCallback(GLFWwindow* window, f64 x, f64 y) {
 
    if (mouse_first) {
	VM2_Set(mouse_last, x, y);
	mouse_first = false;
    }
    
    mouse_delta[0] = x - mouse_last[0];
    mouse_delta[1] = y - mouse_last[1];
    
    
    VM2_Set(mouse_last, x, y);
}

void iVG_ModelArenaInit(u32 size) {
    model_arena.position = 1;
    if (size < 2) size = 2;
    model_arena.size = size;
    model_arena.base = malloc(size*sizeof(Model));
}

u32 iVG_ModelArenaBump() {
    u32 temp = model_arena.position;
    model_arena.position++;
    if (model_arena.position >= model_arena.size) {
	model_arena.size *=2;
	model_arena.base = realloc(model_arena.base, model_arena.size*sizeof(Model));
    }
    return temp;
}

Model* iVG_ModelArenaPointerGet(u32 model_handle) {
    if (model_handle > model_arena.position) {
	assert(false && "Model handle is not valid (too big)");
    }
    return model_arena.base + model_handle;
}

void iVG_ModelArenaDestroy() {
    free(model_arena.base);
}


void iVG_TextureArenaInit(u32 size) {
    texture_arena.position = 1;
    if (size < 2) size = 2;
    texture_arena.size = size;
    texture_arena.base = malloc(size*sizeof(u32));
}

u32 iVG_TextureArenaBump() {
    u32 temp = texture_arena.position;
    texture_arena.position++;
    if (texture_arena.position >= texture_arena.size) {
	texture_arena.size *=2;
	texture_arena.base = realloc(texture_arena.base, texture_arena.size*sizeof(u32));
    }
    return temp;
}
    
u32* iVG_TextureArenaPointerGet(u32 texture_handle) {
    if (texture_handle > texture_arena.position) {
	assert(false && "Texture handle is not valid (too big)");
    }
    return texture_arena.base + texture_handle;
}

void iVG_TextureArenaDestroy() {
    free(texture_arena.base);
}

void iVG_TextureUse(u32 texture_handle) {
    if (texture_handle == texture_current) return;
    texture_current = texture_handle;
    u32* texture = iVG_TextureArenaPointerGet(texture_handle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, *texture);
    iVG_GLUniformIntSet("main_texture", 0);
}
