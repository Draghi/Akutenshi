#version 450

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec3 vTangent;
layout(location = 2) in vec3 vBitangent;
layout(location = 3) in vec3 vNormal;
layout(location = 4) in vec2 vTexCoord;

layout(location = 5) in uvec4 vBoneIndicies;
layout(location = 6) in vec4 vBoneWeights;

layout(location = 0) uniform mat4 uMatProjection;
layout(location = 1) uniform mat4 uMatView;
layout(location = 2) uniform mat4 uMatModel;
layout(location = 7) uniform mat4 uMatMesh;

layout(binding = 0) uniform BoneBlock {
	mat4 transform[26];
} uBones;

out vec3 fPos;
out vec3 fNorm;
out vec2 fTex;
out mat3 fMatTBN;

void main() {
	mat4 pose = uBones.transform[vBoneIndicies[0]] * vBoneWeights[0]
	          + uBones.transform[vBoneIndicies[1]] * vBoneWeights[1]
	          + uBones.transform[vBoneIndicies[2]] * vBoneWeights[2]
	          + uBones.transform[vBoneIndicies[3]] * vBoneWeights[3]; 
	
	gl_Position = uMatProjection * uMatView * uMatModel * uMatMesh * pose * vec4(vPosition, 1);
	fPos = (uMatModel * pose * vec4(vPosition, 1)).xyz;
	fNorm = mat3(uMatMesh) * mat3(pose) * vNormal;
	fTex = vTexCoord;
	fMatTBN = transpose(mat3(uMatModel) * mat3(uMatMesh) * mat3(pose) * mat3(vTangent, vBitangent, vNormal));
}