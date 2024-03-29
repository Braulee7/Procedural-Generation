#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 crnPos;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 light_pos = vec3(5.0, 5.0, 0.0);
    vec3 light_dir = normalize(light_pos - crnPos);
    vec3 normal = -normalize(fragNormal);
    outColor = vec4((max(dot(light_dir, normal), 0.0f) + 0.2) * vec3(fragColor), 1.0);
}