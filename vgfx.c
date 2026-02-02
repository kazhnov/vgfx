#include "vgfx.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#define ARRLEN(x) ((sizeof(x))/(sizeof(x[0])))
typedef uint32_t VBO_t;

static GLFWwindow *window = NULL;
static float background_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
const uint8_t *keys;
static float window_size[2];
static uint32_t shader_program;

const char* vertex_shader_source = "                     \n"
    "#version 330 core                                   \n"
    "layout (location = 0) in vec2 aPos;                 \n"
    "layout (location = 1) in vec4 aColor;               \n"
    "void main()                                         \n"
    "{                                                   \n"
    "    gl_Position = vec4(aPos.x, aPos.y, 1.f, 1.f);   \n"
    "}                                                   \0";


const char* fragment_shader_source = "            \n"
    "#version 330 core                            \n"
    "out vec4 FragColor;                          \n"
    "in vec4 aColor;                              \n"
    "void main()                                  \n"
    "{                                            \n"
    "    FragColor = aColor;                      \n"
    "}                                            \0";

void iVG_RenderFlush();
void iVG_ColorSet(float *color);
void iVG_KeysUpdate();
//SDL_FPoint iVG_FPoint2(float* point);
//SDL_Color  iVG_Color4(float* color);
//SDL_Vertex iVG_Vertex(float* point, float* color, float* texcoord);
//SDL_Vertex iVG_VertexColored(float* point, float* color);
void iVG_SetupOpengl();
void iVG_GLFillRect(float* pos, float* size, float* color);
void iVG_ShadersInit();
void iVG_FramebufferSizeCallback(GLFWwindow *window, int width, int height);
void iVG_InputUpdate();
void iVG_PollEvents();


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

    if (!gladLoadGLLoader((GLADloadproc)(glfwGetProcAddress))) {
	printf("Failed to initiate GLAD\n");
	glfwTerminate();
	exit(1);
    }
    
    iVG_ShadersInit();
}

bool VG_WindowShouldClose() {
    return glfwWindowShouldClose(window);
}

void VG_WindowClose() {}


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


// DRAWING MODES
void VG_DrawingBegin() {
    iVG_InputUpdate();
    VG_Clear(background_color);
}

void VG_DrawingEnd() {
    iVG_RenderFlush();
    iVG_PollEvents();
}


// CLEARING SCREEN
void VG_Clear(float* color) {
    iVG_ColorSet(color);
    glClearColor(color[0], color[1], color[2], color[3]);
    glClear(GL_COLOR_BUFFER_BIT);
}

void VG_BackgroundClear() {
    VG_Clear(background_color);
}


// KEYS
bool VG_KeyPressed(uint64_t key) {
    return keys[key];
}

bool VG_KeyDown(uint64_t key) {
    return keys[key];
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

/* /\* void VG_FillPolygon(float *pos, float r, float angle, uint32_t sides, float* color) { *\/ */
/* /\*     if (sides < 3) return; *\/ */
/* /\*     float center[2]; *\/ */
/* /\*     VM2_Copy(center, pos); *\/ */

/* /\*     SDL_Vertex verts[3*sides]; *\/ */
/* /\*     float prev[2], now[2]; *\/ */
    
/* /\*     prev[0] = center[0] + cosf(angle - 2*3.1415926535f/sides) * r; *\/ */
/* /\*     prev[1] = center[1] + sinf(angle - 2*3.1415926535f/sides) * r; *\/ */
    
/* /\*     for (int i = 0; i < sides; i++) { *\/ */
/* /\* 	now[0] = center[0] + cosf(angle) * r; *\/ */
/* /\* 	now[1] = center[1] + sinf(angle) * r; *\/ */
/* /\* 	verts[3*i]   = iVG_VertexColored(now,    color); *\/ */
/* /\* 	verts[3*i+1] = iVG_VertexColored(center, color); *\/ */
/* /\* 	verts[3*i+2] = iVG_VertexColored(prev,   color); *\/ */
/* /\* 	angle += 2*3.1415926535f/sides; *\/ */
/* /\* 	VM2_Copy(prev, now); *\/ */
/* /\*     } *\/ */

/* /\*     iVG_ColorSet(color); *\/ */
/* /\* //    SDL_RenderGeometry(renderer, NULL, verts, ARRLEN(verts), NULL, 0); *\/ */
/* /\* } *\/ */

void VG_FillCircle(float *pos, float r, float* color) {
//    VG_FillPolygon(pos, r, 0, 32, color);
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

void iVG_GLBufferData(float* vertices, uint32_t size, uint32_t flags) {
    uint32_t VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void iVG_GLDrawTriangles(uint32_t amount) {
    glDrawArrays(GL_TRIANGLES, 0, 3*amount);
}

void iVG_GLVertexArrayBind(uint32_t VAO) {
    glBindVertexArray(VAO);
}

void iVG_GLVertexArrayUnbind() {
    glBindVertexArray(0);
}

uint32_t iVG_GLVertexArrayNew() {
    uint32_t VAO;
    glGenVertexArrays(1, &VAO);
    return VAO;
}

uint32_t iVG_GLVertexArrayDestroy(uint32_t VAO) {
    glDeleteVertexArrays(1, &VAO);
}

void iVG_GLFillRect(float* pos, float* size, float* color) {
    float vertices[] =
	{
	    pos[0],           pos[1],           color[0], color[1], color[2], color[3],
	    pos[0],           pos[1] + size[1], color[0], color[1], color[2], color[3],
	    pos[0] + size[0], pos[1] + size[1], color[0], color[1], color[2], color[3],

	    pos[0],           pos[1],           color[0], color[1], color[2], color[3],	    
	    pos[0] + size[0], pos[1] + size[1], color[0], color[1], color[2], color[3],
	    pos[0] + size[0], pos[1],           color[0], color[1], color[2], color[3],
        };

    uint32_t VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(0));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUseProgram(shader_program);
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
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
	printf("%s\n", info_log);
	exit(1);
    }    

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
    glCompileShader(fragment_shader);

    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);

    if (!success) {
	char info_log[512];
	glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
	printf("%s\n", info_log);
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
	printf("%s\n", info_log);
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
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
	glfwSetWindowShouldClose(window, true);
    }
}

void iVG_PollEvents() {
    glfwPollEvents();
}
