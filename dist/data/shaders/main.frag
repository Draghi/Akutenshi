#version 450

in vec3 fPos;
in vec3 fNorm;
in vec2 fTex;

layout(location = 3) uniform sampler2DArray uTex;

out vec4 fragColour;

void main() {
	float lDiff = max(dot(fNorm, normalize(vec3(0.66, 1.50, -0.75))), 0.1);
	fragColour = vec4(lDiff*texture(uTex, vec3(fTex, fPos.z)).rgb, 1.0);
}
