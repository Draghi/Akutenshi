#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_norm;
layout(location = 2) in vec2 in_tex;

layout(location = 0) uniform mat4 projMat;
layout(location = 1) uniform mat4 viewMat;
layout(location = 2) uniform mat4 modelMat;

out vec3 fPos;
out vec3 fNorm;
out vec2 fTex;

void main() {
	gl_Position = projMat * viewMat * modelMat * vec4(in_position, 1);
	fPos = gl_Position.xyz;
	fNorm = in_norm;
	fTex = in_tex;
}