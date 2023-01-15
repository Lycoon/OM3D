#version 450

#include "utils.glsl"

// G-Buffer for deferred lighting

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

layout (location = 0) out vec3 fragPosition;
layout (location = 1) out vec3 fragNormal;
layout (location = 2) out vec2 fragUV;

layout (std140) uniform Camera
{
	mat4 view;
	mat4 projection;
};

layout (std140) uniform Model
{
	mat4 model;
};

void main()
{
	fragPosition = (model * vec4(position, 1.0)).xyz;
	fragNormal = (model * vec4(normal, 0.0)).xyz;
	fragUV = uv;

	gl_Position = projection * view * model * vec4(position, 1.0);
}
