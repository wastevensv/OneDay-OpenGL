#version 150 
in  vec3 position; 
in  vec2 texcoord;

out vec2 Texcoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main(void) { 
    Texcoord = texcoord;
    gl_Position = proj * view * model * vec4(position, 1.0);
}
