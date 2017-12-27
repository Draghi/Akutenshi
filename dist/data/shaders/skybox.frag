#version 450

in vec3 fTex;

layout(location = 3) uniform samplerCube uTex;

out vec4 fragColour;

void main() {
	fragColour = vec4(texture(uTex, fTex).rgb, 1.0);
}
