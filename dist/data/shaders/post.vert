#version 450

layout(location = 0) in vec2 vPos;

out vec2 fTex;

void main() {
	gl_Position = vec4(vPos, 0.5, 1);
    fTex = (vPos/2 + vec2(0.5,0.5));
}