#version 450

layout(location = 0) in vec2 vPos;

layout(location = 0) uniform mat4 uMatProj;
layout(location = 1) uniform mat4 uMatView;

out vec3 fTex;

void main() {
	gl_Position = vec4(vPos, 1, 1);

    mat4 invProj = inverse(uMatProj);
    vec3 viewPos = (invProj * gl_Position).xyz;
    
    mat3 invView = transpose(mat3(uMatView));
    fTex = (invView * viewPos);
}