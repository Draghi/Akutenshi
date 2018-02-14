#version 450

layout(location = 2) uniform vec3 uCol;

out vec4 fragColour;

void main() {
	fragColour = vec4(uCol, 1);
}