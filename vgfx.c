#include "vgfx.h"
#include "include/vmath/vmath.h"
#include "include/vcolor/vcolor.h"
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
static float background_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
static float window_size[2];
static double current_time;
static double previous_time;
static bool vsync;
static uint32_t shader_program;


static Camera camera;
static Light light;
static float ambient_color[3];

static bool mouse_first = true;
static float mouse_last[2];
static float mouse_delta[2];

static bool keys_just_pressed[512];
static bool keys_pressed[512];
void iVG_KeysJustPressedClear();

const uint32_t dims = 3;
const uint32_t normals = 3;
const uint32_t uvs = 2;
const uint32_t vert_size = dims + normals + uvs;

void iVG_RenderFlush();
void iVG_KeysUpdate();
void iVG_SetupOpengl();
void iVG_FramebufferSizeCallback(GLFWwindow *window, int width, int height);
void iVG_MouseCallback(GLFWwindow* window, double x, double y);
void iVG_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void iVG_InputUpdate();
void iVG_PollEvents();

char* iVG_FileLoadToString(const char* path);

typedef struct {
    uint32_t VAO;
    uint32_t index_count;
    uint32_t shader;
    float color[3];
} Model;

// MODELARENA
typedef struct {
    Model* base;
    uint32_t position;
    uint32_t size;
} ModelArena;

static ModelArena model_arena;

void     iVG_ModelArenaInit(uint32_t size);
uint32_t iVG_ModelArenaBump();
Model*   iVG_ModelArenaPointerGet(uint32_t model_handle);
void     iVG_ModelArenaDestroy();

// BUFFERING DATA
typedef uint32_t VAO_t;
typedef uint32_t VBO_t;
VAO_t iVG_GLVertexArrayNew();
void  iVG_GLVertexArrayBind(VAO_t VAO);
void  iVG_GLBufferData(uint32_t pointer, float* vertices, uint32_t arr_size, uint32_t flags, uint32_t stride);
void  iVG_GLDrawTriangles(VAO_t amount);
void  iVG_GLVertexArrayDestroy(VAO_t VAO);
void  iVG_GLModelRender(Model *VAO);
uint32_t iVG_GLLoadVerticesIndexed(Vertex* vertices, uint32_t vcount, uint32_t* indices, uint32_t icount);
void  iVG_GLRenderVerticesIndexed(Vertex* vertices, uint32_t vcound, uint32_t *indices, uint32_t icount);
void  iVG_GLTransformSet(float* matrix);

void  iVG_GLCameraUpdate();
void  iVG_GLLightUpdate();
void  iVG_GLPerspectiveUpdate();

void  iVG_GLUniformVec3Set(char* name, float* vec);



// INITIALIZATION AND CLOSING
void VG_WindowOpen(char* name, float* size, uint32_t flags) {
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
    
    float mat[16] = VM44_IDENTITY;
    iVG_GLTransformSet(mat);
    iVG_ModelArenaInit(1024);
}

bool VG_WindowShouldClose() {
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

void VG_VSyncSet(bool value) {
    vsync = value;
    glfwSwapInterval(value);
}

bool VG_VSyncGet() {
    return vsync;
}

// BACKGROUND COLOR
void VG_BackgroundColorSet(float* in) {
    VRGBA_Copy(background_color, in);
}

void VG_BackgroundColorGet(float* out) {
    VRGBA_Copy(out, background_color);
}


// WINDOW SIZE
void VG_WindowSizeGet(float* out) {
    VM2_Copy(out, window_size);
}

void VG_WindowSizeUpdate() {
    VG_WindowSizeGet(window_size);
    glViewport(0, 0, window_size[0], window_size[1]);
}


// FPS
double VG_FPSGet() {
    return 1./VG_DeltaTimeGet();
}

double VG_DeltaTimeGet() {
    return current_time - previous_time;
}

double VG_TimeGet() {
    return current_time;
}

// CAMERA

void VG_CameraPositionSet(float *pos) {
    VM3_Copy(camera.position, pos);
}

void VG_CameraPositionGet(float *pos) {
    VM3_Copy(pos, camera.position);
}

void VG_CameraRotationSet(float *rot) {
    VM3_Copy(camera.rotation, rot);
}

void VG_CameraRotationGet(float *rot) {
    VM3_Copy(rot, camera.rotation);
}

void VG_CameraForwardGet(float* out) {
    VM3_Set(out, 0, 0, -1);
    VM3_RotateY(out, camera.rotation[1]);
}

void VG_CameraRightGet(float* out) {
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
Light *VG_LightGet() {
    return &light;
}

void VG_LightDirectionSet(float *dir) {
    VM3_Copy(light.direction, dir);
    light.type = LIGHT_TYPE_DIRECTIONAL;
}

void VG_LightDirectionGet(float* out) {
    VM3_Copy(out, light.direction);
}

void VG_LightPositionSet(float* dir) {
    VM3_Copy(light.direction, dir);
    light.type = LIGHT_TYPE_POINT;
}

void VG_LightPositionGet(float* out) {
    VM3_Copy(out, light.direction);
}


void VG_LightColorSet(float* color) {
    VM3_Copy(light.color, color);
}

void VG_LightColorGet(float* out) {
    VM3_Copy(out, light.color);
}

void VG_LightAmbientColorSet(float* color) {
    VM3_Copy(ambient_color, color);
}

void VG_LightAmbientColorGet(float* out) {
    VM3_Copy(out, ambient_color);
}



// DRAWING MODES
void VG_DrawingBegin() {
    iVG_KeysJustPressedClear();
    iVG_InputUpdate();
    iVG_GLCameraUpdate();
    iVG_GLLightUpdate();
    iVG_GLPerspectiveUpdate();
    VG_Clear(background_color);
    previous_time = current_time;
    current_time = glfwGetTime();
}

void VG_DrawingEnd() {
    iVG_RenderFlush();
    
}


// CLEARING SCREEN
void VG_Clear(float* color) {
    glClearColor(color[0], color[1], color[2], color[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
}

void VG_BackgroundClear() {
    VG_Clear(background_color);
}

// KEYS
bool VG_KeyPressed(uint64_t key) {
    return keys_just_pressed[key];
}

bool VG_KeyDown(uint64_t key) {
    return glfwGetKey(window, key) == GLFW_PRESS;
}


//MOUSE
void VG_MouseDeltaGet(float* out) {
    VM2_Copy(out, mouse_delta);
}

void VG_MouseGet(float* out) {
    VM2_Copy(out, mouse_last);
}

// MODEL
uint32_t VG_ModelNew(char* path, uint32_t shader) {
    uint32_t model_handle = iVG_ModelArenaBump();
    Model* model = iVG_ModelArenaPointerGet(model_handle);
    Mesh* mesh = malloc(sizeof(Mesh));
    VMESH_LoadObj(mesh, path);
    model->VAO = iVG_GLLoadVerticesIndexed(mesh->vertices, mesh->vertex_count,
				     mesh->indices, mesh->index_count);
    model->index_count = mesh->index_count;
    model->shader = shader;
    VM3_Set(model->color, 0, 0, 0);
    return model_handle;
}

void VG_ModelDrawAt(uint32_t model_handle, float pos[static 3], float size[static 3]) {
    Model* model = iVG_ModelArenaPointerGet(model_handle);
    float transform[16] = VM44_IDENTITY;
    VM44_Scale(transform, size);
    VM44_Translate(transform, pos);

    VG_ShaderUse(model->shader);
    iVG_GLTransformSet(transform);
    iVG_GLUniformVec3Set("material.color", model->color);
    
    iVG_GLModelRender(model);
}

void VG_ModelColorSet(uint32_t model_handle, float color[static 3]) {
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

void iVG_GLDrawTriangles(uint32_t amount) {
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


void iVG_GLBufferIndices(uint32_t* indices, uint32_t arr_len, uint32_t flags) {
    uint32_t EBO;
    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, arr_len*sizeof(uint32_t), indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void iVG_GLBufferVertices(Vertex* vertices, uint32_t count) {
    uint32_t VBO;
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

uint32_t iVG_GLLoadVerticesIndexed(Vertex* vertices, uint32_t vcount, uint32_t* indices, uint32_t icount) {
    uint32_t VAO = iVG_GLVertexArrayNew();
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
    uint32_t length = ftell(file);
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
void VG_ShaderUse(uint32_t shader) {
    shader_program = shader;
    iVG_GLCameraUpdate();
    iVG_GLLightUpdate();
    iVG_GLPerspectiveUpdate();
    glUseProgram(shader_program);
}

uint32_t VG_ShaderLoad(const char* vertex_path, const char* fragment_path) {
    uint32_t vertex_shader = glCreateShader(GL_VERTEX_SHADER);

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

    uint32_t fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
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

void iVG_VertexColoredGet(float* point, float* color, float* out) {
    VM3_Copy(out, point);
    VRGBA_Copy(out+dims, color);
}

void  iVG_GLTransformSet(float* matrix) {
    uint32_t transform_loc = glGetUniformLocation(shader_program, "model");
    glUniformMatrix4fv(transform_loc, 1, GL_TRUE, matrix);
}


void iVG_GLPerspectiveUpdate() {
    uint32_t perspective_loc = glGetUniformLocation(shader_program, "projection");
    float matrix[16];
    float size[2];
    VG_WindowSizeGet(size);
    VM44_ProjectionPerspective(matrix, camera.fov, size[0]/size[1], 0.1, 100);
    glUniformMatrix4fv(perspective_loc, 1, GL_TRUE, matrix);
}

void iVG_GLUniformVec3Set(char* name, float* vec) {
    uint32_t loc = glGetUniformLocation(shader_program, name);
    glUniform3fv(loc, 1, vec);
}

void iVG_GLUniformIntSet(char *name, int i) {
    uint32_t loc = glGetUniformLocation(shader_program, name);
    glUniform1i(loc, i);
}

void iVG_GLCameraUpdate() {
    uint32_t camera_loc = glGetUniformLocation(shader_program, "view");
    float matrix[16];
    float out[16];
    VM44_V3A3(camera.position, camera.rotation, matrix);
    VM44_InverseO(matrix, out);
    glUniformMatrix4fv(camera_loc, 1, GL_TRUE, out);
    iVG_GLUniformVec3Set("cameraPos", camera.position);
}

void iVG_GLLightUpdate() {
    iVG_GLUniformVec3Set("light.dir", light.direction);
    iVG_GLUniformVec3Set("light.color", light.color);
    iVG_GLUniformIntSet("light.type", light.type);
    iVG_GLUniformVec3Set("ambient", ambient_color);
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
    memset(keys_just_pressed, 0, 512*sizeof(bool));
}

void iVG_MouseCallback(GLFWwindow* window, double x, double y) {
 
    if (mouse_first) {
	VM2_Set(mouse_last, x, y);
	mouse_first = false;
    }
    
    mouse_delta[0] = x - mouse_last[0];
    mouse_delta[1] = y - mouse_last[1];
    
    
    VM2_Set(mouse_last, x, y);
}

void iVG_ModelArenaInit(uint32_t size) {
    model_arena.position = 1;
    if (size < 2) size = 2;
    model_arena.size = size;
    model_arena.base = malloc(size*sizeof(uint32_t));
}

uint32_t iVG_ModelArenaBump() {
    uint32_t temp = model_arena.position;
    model_arena.position++;
    if (model_arena.position >= model_arena.size) {
	model_arena.size *=2;
	model_arena.base = realloc(model_arena.base, model_arena.size*sizeof(uint32_t));
    }
    return temp;
}

Model* iVG_ModelArenaPointerGet(uint32_t model_handle) {
    if (model_handle > model_arena.position) {
	assert(false && "Model handle is not valid (too big)");
    }
    return model_arena.base + model_handle;
}

void iVG_ModelArenaDestroy() {
    free(model_arena.base);
}
