#version 450

// /////////// //
// // Input // //
// /////////// //

layout(location =  0) in vec3 vertexPosition;
layout(location =  6) in vec2 vertexTextureCoord[4];
layout(location = 10) in vec4 vertexColour;

// ////////////// //
// // Uniforms // //
// ////////////// //

layout(binding = 0, std140) uniform Material {
	vec4 baseColour;
	float metallicFactor;
	float roughnessFactor;
	vec3 emmisiveFactor;
	
	uint baseCoordSet;
	uint metalRoughnessCoordSet;
	uint normalCoordSet;
	uint occlusionCoordSet;
	uint emissiveCoordSet;
	
	uint alphaMode;
	float alphaCutoff;
} material;

layout(location = 0) uniform mat4 matViewProj;
layout(location = 1) uniform mat4 matProjView;

layout(location = 2) uniform mat4 matWorldView;
layout(location = 3) uniform mat4 matViewWorld;

layout(location = 4) uniform mat4 matModelWorld;
layout(location = 5) uniform mat4 matWorldModel;

// //////////// //
// // Output // //
// //////////// //

out vec2 fragBaseTexCoord;
out float fragAlpha;
out float fragDepth;

void main() {
	vec4 viewPos = matWorldView * matModelWorld * vec4(vertexPosition, 1);
	gl_Position = matViewProj * viewPos;
	
	fragAlpha = vertexColour.a;
	fragBaseTexCoord = vertexTextureCoord[material.baseCoordSet];
	fragDepth = viewPos.z;
}

