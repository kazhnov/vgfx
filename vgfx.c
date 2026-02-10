#include "vgfx.h"
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
static uint32_t shader_program;
static double current_time;
static double previous_time;
static bool vsync;

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

const char* vertex_shader_source = "                     \n"
    "#version 330 core                                   \n"
    "layout (location = 0) in vec3 aPos;                 \n"
    "layout (location = 1) in vec3 aNormal;               \n"
    "layout (location = 2) in vec2 aTex;                 \n"
    "uniform mat4 view;                                \n"
    "uniform mat4 model;                             \n"
    "uniform mat4 projection;                           \n"
    "out vec3 bNormal;                                    \n"
    "out vec3 bPos;                                    \n"
    "void main()                                         \n"
    "{                                                   \n"
    "    bPos = vec3(model*vec4(aPos, 1.0));                  \n"
    "    bNormal = mat3(transpose(inverse(model))) * aNormal; \n"
    "    gl_Position = projection*view*model*vec4(aPos, 1.f);\n"
    "}                                                   \0";

const char* fragment_shader_source = "            \n"
    "#version 330 core                            \n"
    "out vec4 FragColor;                          \n"
    "in vec3 bNormal;                             \n"
    "in vec3 bPos;                                \n"
    "uniform vec3 cameraPos;                      \n"
    "uniform struct {                             \n"
    "  vec3 dir; vec3 color;int type;             \n"
    "} light;                                     \n"
    "uniform vec3 ambient;                        \n"
    "void main()                                  \n"
    "{                                            \n"
    "         float refl = 0.8;                                \n"
    "         vec3 normal = normalize(bNormal);                \n"
    "         vec3 direction;                                  \n"
    "         float light_factor = 1;                          \n"
    "         if (light.type == 0) {                           \n"
    "             direction = -normalize(light.dir);           \n"
    "         } else {                                         \n"
    "             direction = -normalize(bPos - light.dir);     \n"
    "             float distance = length(bPos - light.dir);    \n"
    "             light_factor = 1.0/(1.0 + 0.1*distance + 0.01*distance*distance);\n"
    "         }                                                \n"
    "         float diff = max(dot(direction, normal)*light_factor, 0.0);   \n"
    "         vec3 diffuse = diff*light.color;                 \n"
    "         vec3 result  = refl*(ambient+diffuse);           \n"
    "         FragColor = vec4(result, 1.0);                   \n"
    "}                                                         \0";

void iVG_RenderFlush();
void iVG_KeysUpdate();
void iVG_SetupOpengl();
void iVG_ShadersInit();
void iVG_FramebufferSizeCallback(GLFWwindow *window, int width, int height);
void iVG_MouseCallback(GLFWwindow* window, double x, double y);
void iVG_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void iVG_InputUpdate();
void iVG_PollEvents();

// MODELARENA
typedef struct {
    Mesh* base;
    uint32_t position;
    uint32_t size;
} ModelArena;

static ModelArena model_arena;

void     iVG_ModelArenaInit(uint32_t size);
uint32_t iVG_ModelArenaBump();
Mesh*    iVG_ModelArenaPointerGet(uint32_t model_handle);
void     iVG_ModelArenaDestroy();

// BUFFERING DATA
typedef uint32_t VAO_t;
typedef uint32_t VBO_t;
VAO_t iVG_GLVertexArrayNew();
void  iVG_GLVertexArrayBind(VAO_t VAO);
void  iVG_GLBufferData(uint32_t pointer, float* vertices, uint32_t arr_size, uint32_t flags, uint32_t stride);
void  iVG_GLDrawTriangles(VAO_t amount);
void  iVG_GLVertexArrayDestroy(VAO_t VAO);
void  iVG_GLRenderVerticesIndexed(Vertex* vertices, uint32_t vcound, uint32_t *indices, uint32_t icount);
void  iVG_GLTransformSet(float* matrix);
void  iVG_GLCameraUpdate();
void  iVG_GLLightUpdate();
void  iVG_GLUniformVec3Set(char* name, float* vec);
void  iVG_GLPerspectiveUpdate();


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
    
    iVG_ShadersInit();
    current_time = glfwGetTime();

    camera.fov = V_PI/2;
    
    float mat[16] = VM44_IDENTITY;
    glUseProgram(shader_program);
    
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
uint32_t VG_ModelNew(char* path) {
    uint32_t model_handle = iVG_ModelArenaBump();
    Mesh* mesh = iVG_ModelArenaPointerGet(model_handle);
    VMESH_LoadObj(mesh, path);
    return model_handle;
}

void VG_ModelDrawAt(uint32_t model_handle, float pos[static 3], float size[static 3]) {
    float transform[16] = VM44_IDENTITY;
    VM44_Scale(transform, size);
    VM44_Translate(transform, pos);
    iVG_GLTransformSet(transform);
    Mesh* mesh = iVG_ModelArenaPointerGet(model_handle);
    iVG_GLRenderVerticesIndexed(mesh->vertices, mesh->vertex_count,
				mesh->indices, mesh->index_count);
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

void  iVG_GLRenderVerticesIndexed(Vertex* vertices, uint32_t vcount, uint32_t *indices, uint32_t icount) {
    uint32_t VAO = iVG_GLVertexArrayNew();
    iVG_GLVertexArrayBind(VAO);
    
    iVG_GLBufferVertices(vertices, vcount);
    
    iVG_GLBufferIndices(indices, icount, 0);

    glUseProgram(shader_program);
    
    glDrawElements(GL_TRIANGLES, icount, GL_UNSIGNED_INT, NULL);

    iVG_GLVertexArrayBind(0);
    iVG_GLVertexArrayDestroy(VAO);    
}

void iVG_ShadersInit() {
    int success;
    uint32_t vertex_shader;
    uint32_t fragment_shader;

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
    glCompileShader(vertex_shader);
    
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);

    if (!success) {
	char info_log[512];
	glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
	printf("VERTEX SHADER ERROR: %s\n", info_log);
	exit(1);
    }    

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);

    if (!success) {
	char info_log[512];
	glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
	printf("FRAGMENT SHADER ERROR: %s\n", info_log);
	exit(1);
    }

    shader_program = glCreateProgram();
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
    model_arena.base = malloc(size*sizeof(Mesh));
}

uint32_t iVG_ModelArenaBump() {
    uint32_t temp = model_arena.position;
    model_arena.position++;
    if (model_arena.position >= model_arena.size) {
	model_arena.size *=2;
	model_arena.base = realloc(model_arena.base, model_arena.size*sizeof(Mesh));
    }
    return temp;
}

Mesh* iVG_ModelArenaPointerGet(uint32_t model_handle) {
    if (model_handle > model_arena.position) {
	assert(false && "Model handle is not valid (too big)");
    }
    return model_arena.base + model_handle;
}

void iVG_ModelArenaDestroy() {
    free(model_arena.base);
}
