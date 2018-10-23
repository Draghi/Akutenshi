#version 450

// ///////////////// //
// // Definitions // //
// ///////////////// //

struct Texture {
	uint coordSet;
	Sampler3D sampler;
};

// /////////// //
// // Input // //
// /////////// //

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexTangent;
layout(location = 2) in vec3 vertexBitangent;
layout(location = 3) in vec3 vertexNormal;

layout(location = 4) in uvec4 vertexBoneIndicies;
layout(location = 5) in vec4 vertexBoneWeights;

layout(location = 6) in vec2[4] vertexTextureCoord; // 7, 8, 9

layout(location = 10) in vec4 vertexColour;

// ////////////// //
// // Uniforms // //
// ////////////// //

buffer mat4 Bones {
	mat4 transform[];
} bones;

uniform Material {
	vec4 baseColour;
	float metallicFactor;
	float roughnessFactor;
	vec3 emmisiveFactor;
	
	Texture baseTexture;
	Texture metalRoughnessTexture;
	Texture normalTexture;
	Texture occlusionTexture;
	Texture emissiveTexture;
	
	uint alphaMode;
	float alphaCutoff;
} material;








