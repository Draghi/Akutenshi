#version 450

in vec3 fPos;
in vec3 fNorm;
in vec2 fTex;

layout(location = 3) uniform sampler2D uTex;

out vec4 fragColour;

void main() {
	float lDiff = max(dot(fNorm, normalize(vec3(0.66, 1.50, -0.75))), 0.1);
	fragColour = vec4(lDiff*texture(uTex, fTex).rgb*vec3(fTex, 1), 1.0);
}
