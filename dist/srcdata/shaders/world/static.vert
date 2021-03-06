#version 450

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vTangent;
layout(location = 2) in vec3 vBitangent;
layout(location = 3) in vec3 vNormal;
layout(location = 4) in vec2 vTexCoord;

layout(location = 0) uniform mat4 uMatProjection;
layout(location = 1) uniform mat4 uMatView;
layout(location = 2) uniform mat4 uMatModel;
layout(location = 7) uniform mat4 uMatMesh;

out vec3 fPos;
out vec3 fNorm;
out vec2 fTex;
out mat3 fMatTBN;

void main() {
	gl_Position = uMatProjection * uMatView * uMatModel * uMatMesh * vec4(vPosition, 1);
	fPos = (uMatModel * vec4(vPosition, 1)).xyz;
	fNorm = mat3(uMatMesh) * vNormal;
	fTex = vTexCoord;
	fMatTBN = transpose(mat3(uMatModel) * mat3(uMatMesh) * mat3(vTangent, vBitangent, vNormal));
}