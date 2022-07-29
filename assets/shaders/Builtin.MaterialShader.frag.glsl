#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform LocalUniformObject {
    vec4 diffuseColor;
} localUniformObject;

// Samplers.
layout(set = 1, binding = 1) uniform sampler2D diffuseSampler;

// Data transfer object.
layout(location = 1) in struct DataTransferObject {
    vec2 texCoord;
} inDataTransferObject;

void main() {
    outColor = localUniformObject.diffuseColor * texture(diffuseSampler, inDataTransferObject.texCoord);
}
