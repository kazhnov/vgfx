#include "vgfx.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#define ARRLEN(x) ((sizeof(x))/(sizeof(x[0])))
static GLFWwindow *window = NULL;
static float background_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
const uint8_t *keys;
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

const uint32_t dims = 3;
const uint32_t normals = 4;
const uint32_t vert_size = dims;
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
#if 1
    "         FragColor = vec4(normalize(bNormal), 1.0);                   \n"
#else
    "         vec3 direction;                                  \n"
    "         if (light.type == 0) {                           \n"
    "             direction = -normalize(light.dir);           \n"
    "         } else {                                         \n"
    "             direction = -normalize(bPos - light.dir);     \n"
    "         }                                                \n"
    "         float diff = max(dot(direction, normal), 0.0);   \n"
    "         vec3 diffuse = diff*light.color;                 \n"
    "         vec3 result  = refl*(ambient+diffuse);           \n"
    "         FragColor = vec4(result, 1.0);                   \n"
#endif
    "}                                                         \0";

void iVG_RenderFlush();
void iVG_ColorSet(float *color);
void iVG_KeysUpdate();
void iVG_VertexColoredGet(float* point, float* color, float* out);
void iVG_SetupOpengl();
void iVG_GLFillRect(float* pos, float* size, float* color);
void iVG_ShadersInit();
void iVG_FramebufferSizeCallback(GLFWwindow *window, int width, int height);
void iVG_MouseCallback(GLFWwindow* window, double x, double y);
void iVG_InputUpdate();
void iVG_PollEvents();


// BUFFERING DATA
typedef uint32_t VAO_t;
typedef uint32_t VBO_t;
VAO_t iVG_GLVertexArrayNew();
void  iVG_GLVertexArrayBind(VAO_t VAO);
void  iVG_GLBufferData(uint32_t pointer, float* vertices, uint32_t arr_size, uint32_t flags, uint32_t stride);
void  iVG_Gl_BufferVertices(Vertex* vertices, uint32_t arr_size);
void  iVG_GLDrawTriangles(VAO_t amount);
void  iVG_GLVertexArrayDestroy(VAO_t VAO);
void  iVG_GLRenderVertices(float* vertices, uint32_t size);
void  iVG_GLRenderVerticesNormalsIndices(float* vertices, uint32_t vcount, float* normals, uint32_t *indices, uint32_t icount);
void  iVG_GLRenderVerticesIndexed(Vertex* vertices, uint32_t vcound, uint32_t *indices, uint32_t icount);
void  iVG_GLSetTransform(float* matrix);
void  iVG_GLCameraUpdate();
void iVG_GLLightUpdate();
void iVG_GLUniformVec3Set(char* name, float* vec);
void iVG_GLPerspectiveUpdate();
void iVG_EntityGetMatrix(Entity* entity, float* matrix);


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
//    float p[3] = {0, 0, -2};
//    VM44_Translate(mat, p);
    glUseProgram(shader_program);
    
    iVG_GLSetTransform(mat);
}

bool VG_WindowShouldClose() {
    return glfwWindowShouldClose(window);
}

void VG_WindowClose() {}


void VG_WindowTitleSet(char* new) {
    glfwSetWindowTitle(window, new);
}

void VG_WindowTitleGet(char* out) {
    printf("not implemented WindowTitleGet\n");
    exit(1);
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
    iVG_ColorSet(color);
    glClearColor(color[0], color[1], color[2], color[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
}

void VG_BackgroundClear() {
    VG_Clear(background_color);
}

// KEYS
bool VG_KeyPressed(uint64_t key) {
    return glfwGetKey(window, key) == GLFW_PRESS;
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

// ENTITIES
struct Entity {
    Mesh* mesh;
    float pos[3];
    float size;
};

Entity* VG_EntityCreate() {
    return calloc(1, sizeof(Entity));
}

Entity* VG_EntityCreateWithMesh(Mesh* mesh) {
    Entity* entity = VG_EntityCreate();
    VG_EntityMeshSet(entity, mesh);
    return entity;
}

void VG_EntityDestroy(Entity* entity) {
    free(entity);
}

void VG_EntityMeshSet(Entity* entity, Mesh* mesh) {
    printf("setting entity mesh\n");
    entity->mesh = mesh;
    printf("mesh is set\n");
}

void VG_EntityPosSet(Entity* entity, float* in) {
    VM3_Copy(entity->pos, in);
}

void VG_EntityPosGet(Entity* entity, float* out) {
    VM3_Copy(out, entity->pos);
}


float VG_EntitySizeGet(Entity* entity) {
    return entity->size;
}

void VG_EntitySizeSet(Entity* entity, float size) {
    entity->size = size;
}


void VG_EntityDraw(Entity* entity) {
    float transform[16];
    iVG_EntityGetMatrix(entity, transform);
    iVG_GLSetTransform(transform);

    iVG_GLRenderVerticesIndexed(VMESH_Vertices(entity->mesh),
				VMESH_VertexCount(entity->mesh),
				VMESH_Indices(entity->mesh),
				VMESH_IndicesCount(entity->mesh)
	);
}

// DRAWING SHAPES
void VG_FillRect(float* pos, float* size, float* color) {
    iVG_ColorSet(color);
    iVG_GLFillRect(pos, size, color);
}

void VG_FillRectCentered(float* pos, float* size, float* color) {
    float realpos[2];
    float half[2];
    VM2_ScaleO(size, 0.5, half);
    VM2_SubO(pos, half, realpos);
    VG_FillRect(realpos, size, color);
}

void VG_DrawCircle(float* pos, float r, float* color);

void VG_FillPolygon(float *pos, float r, float angle, uint32_t sides, float* color) {
    if (sides < 3) return;
    float center[2];
    VM2_Copy(center, pos);

    float verts[vert_size*sides*3];
    float prev[2], now[2];
    
    prev[0] = center[0] + cosf(angle - 2*3.1415926535f/sides) * r;
    prev[1] = center[1] + sinf(angle - 2*3.1415926535f/sides) * r;
    
    for (int i = 0; i < sides; i++) {
	now[0] = center[0] + cosf(angle) * r;
	now[1] = center[1] + sinf(angle) * r;
	iVG_VertexColoredGet(now,    color, &verts[vert_size*(i*3+0)]);
	iVG_VertexColoredGet(prev,   color, &verts[vert_size*(i*3+1)]);
	iVG_VertexColoredGet(center, color, &verts[vert_size*(i*3+2)]);
	angle += 2*3.1415926535f/sides;
	VM2_Copy(prev, now);
    }
    
    iVG_GLRenderVertices(verts, sizeof(verts));
}

void VG_FillCircle(float *pos, float r, float* color) {
    VG_FillPolygon(pos, r, 0, 32, color);
}


void VG_DrawLine(float* from, float* to, float* color) {
    iVG_ColorSet(color);
    //SDL_RenderLine(renderer, from[0], from[1], to[0], to[1]);
}

void VG_DrawLines(float* points, uint32_t amount, float* color) {
    iVG_ColorSet(color);
    for (int i = 0; i < amount - 2; i += 2) {
	//SDL_RenderLine(renderer, points[i], points[i+1], points[i+2], points[i+3]);
    }
}


// INTERNALS
void iVG_RenderFlush() {
    glfwSwapBuffers(window);
}

void iVG_ColorSet(float *color) {
    
}

void iVG_KeysUpdate() {
//    keys = VG_KeysGet(NULL);
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


void iVG_GLFillRect(float* pos, float* size, float* color) {
    float vertices[] =
	{
	    pos[0],           pos[1],           0, color[0], color[1], color[2], color[3],
	    pos[0],           pos[1] + size[1], 0, color[0], color[1], color[2], color[3],
	    pos[0] + size[0], pos[1] + size[1], 0, color[0], color[1], color[2], color[3],

	    pos[0],           pos[1],           0, color[0], color[1], color[2], color[3],	    
	    pos[0] + size[0], pos[1] + size[1], 0, color[0], color[1], color[2], color[3],
	    pos[0] + size[0], pos[1],           0, color[0], color[1], color[2], color[3],
        };

    iVG_GLRenderVertices(vertices, sizeof(vertices));
}

void iVG_GLRenderVertices(float *vertices, uint32_t size) {
    uint32_t VAO = iVG_GLVertexArrayNew();
    iVG_GLVertexArrayBind(VAO);

    iVG_GLBufferData(0, vertices, size, 0, vert_size);
    
    glUseProgram(shader_program);
    iVG_GLDrawTriangles(size/(sizeof(float)*vert_size));

    iVG_GLVertexArrayDestroy(VAO);
}

void iVG_GLBufferData(uint32_t attrib_pointer, float* data, uint32_t count, uint32_t flags, uint32_t stride) {
    uint32_t VBO;
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, count*stride*sizeof(float), data, GL_STATIC_DRAW);

    glVertexAttribPointer(attrib_pointer, stride, GL_FLOAT, GL_FALSE, stride*sizeof(float), (void*)(0));
    glEnableVertexAttribArray(attrib_pointer);

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
#if 0
    for (int i = 0; i < 20; i++) {
//	int j = indices[i];
	printf("vertex: %f %f %f\n",
	       vertices[i].pos[0],
	       vertices[i].pos[1],
	       vertices[i].pos[2]);
	printf("index: %d\n", indices[i]);
    }
    exit(1);
#endif
    printf("vcount: %u, icount: %u\n", vcount, icount);
    uint32_t VAO = iVG_GLVertexArrayNew();
    iVG_GLVertexArrayBind(VAO);
    
    iVG_GLBufferVertices(vertices, vcount);
    
    iVG_GLBufferIndices(indices, icount, 0);

    glUseProgram(shader_program);
    
    glDrawElements(GL_TRIANGLES, icount, GL_UNSIGNED_INT, NULL);

    iVG_GLVertexArrayBind(0);
    iVG_GLVertexArrayDestroy(VAO);
    
}

void iVG_GLRenderVerticesNormalsIndices(float* vertices, uint32_t vcount, float* normals, uint32_t *indices, uint32_t icount) {
    uint32_t VAO = iVG_GLVertexArrayNew();
    iVG_GLVertexArrayBind(VAO);
    iVG_GLBufferData(0, vertices, vcount, 0, 3);

    if (normals) {
/*	for (int i = 0; i < 10; i++) {
	    printf("%f %f %f\n", normals[3*i], normals[3*i+1], normals[3*i+2]);
	}
*/	
	iVG_GLBufferData(1, normals, vcount, 0, 3);
    }
    
    iVG_GLBufferIndices(indices, icount*sizeof(uint32_t), 0);

    glUseProgram(shader_program);
    
    glDrawElements(GL_TRIANGLES, icount, GL_UNSIGNED_INT, NULL);

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

void  iVG_GLSetTransform(float* matrix) {
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

void iVG_EntityGetMatrix(Entity* entity, float* matrix) {
    float temp[16] = VM44_IDENTITY;
    VM44_Scale(temp, (float[]){entity->size, entity->size, entity->size});
    VM44_Translate(temp, entity->pos);
    VM44_Copy(matrix, temp);
}

void iVG_MouseCallback(GLFWwindow* window, double x, double y) {
 
    if (mouse_first) {
	VM2_Set(mouse_last, x, y);
	mouse_first = false;
    }
    
    mouse_delta[0] = x - mouse_last[0];
    mouse_delta[1] = y - mouse_last[1];
    printf("x: %f, y: %f\n", mouse_delta[0], mouse_delta[1]);
    
    
    VM2_Set(mouse_last, x, y);
}
