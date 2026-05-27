#version 450 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 in_tex_coord;
layout(binding = 0) uniform MVP_uniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} mvp;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec2 out_tex_coord;

void main() {
//    gl_Position = mvp.proj * mvp.view * mvp.model * vec4(inPosition, 1.0);
    gl_Position = vec4(inPosition, 1.0);
    fragColor = vec4(inColor, 1.0);
    out_tex_coord = in_tex_coord;
}