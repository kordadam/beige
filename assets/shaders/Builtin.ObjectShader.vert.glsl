#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;

layout(set = 0, binding = 0) uniform GlobalUniformObject {
    mat4 projection;
    mat4 view;
} globalUniformObject;

void main() {
    gl_Position = globalUniformObject.projection * globalUniformObject.view * vec4(inPosition, 1.0);
}
