#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTex;
layout (location = 3) in mat4 aInstance;

uniform mat4 view;
uniform mat4 projection;

out vec3 bNormal;
out vec3 bPos;
out vec2 bTex;

void main()
{
    bPos = (aInstance*vec4(aPos, 1.0)).xyz;
    bNormal = mat3(transpose(inverse(aInstance))) * aNormal;
    bTex = aTex;
    gl_Position = projection*view*aInstance*vec4(aPos, 1.0);
}
