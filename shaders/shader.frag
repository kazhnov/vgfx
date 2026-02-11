#version 330 core
out vec4 FragColor;
in vec3 bNormal;
in vec3 bPos;

uniform vec3 cameraPos;
uniform struct {
    vec3 dir;
    vec3 color;
    int type;
} light;
uniform vec3 ambient;
uniform struct {
    vec3 color;
} material;

void main()
{
    float refl = 0.8;
    vec3 normal = normalize(bNormal);
    vec3 direction;
    float light_factor = 1;
    if (light.type == 0) {
	direction = -normalize(light.dir);
    } else {
	direction = -normalize(bPos - light.dir);
	float distance = length(bPos - light.dir);
	light_factor = 1.0/(1.0 + distance*distance);
    }
    float diff = max(dot(direction, normal), 0.0);
    vec3 diffuse = diff*light_factor*light.color;
    vec3 result  = refl*(ambient+diffuse);
    FragColor = vec4(result, 1.0);
    FragColor = pow(FragColor, vec4(vec3(1./2.2), 1));
}
