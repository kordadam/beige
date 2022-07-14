#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;

layout(set = 0, binding = 0) uniform GlobalUniformObject {
    mat4 projection;
    mat4 view;
} globalUniformObject;

layout(push_constant) uniform PushConstants {
    // Only guaranteed a total of 128 bytes.
    mat4 model; // 64 bytes
} pushConstants;

void main() {
    gl_Position = globalUniformObject.projection * globalUniformObject.view * pushConstants.model * vec4(inPosition, 1.0);
}
