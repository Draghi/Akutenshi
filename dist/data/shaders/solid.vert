#version 450

layout(location = 0) in vec3 vPos;

layout(location = 0) uniform mat4 uMatProj;
layout(location = 1) uniform mat4 uMatView;

void main() {
	gl_Position = uMatProj * uMatView * vec4(vPos, 1);
}