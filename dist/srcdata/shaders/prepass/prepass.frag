#version 450

layout(early_fragment_tests) in;

// /////////// //
// // Input // //
// /////////// //

in vec2 fragBaseTexCoord;
in float fragAlpha;
in float fragDepth;

// ////////////// //
// // Uniforms // //
// ////////////// //

// Alpha-test
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

layout(location = 6) uniform sampler2D baseTexture;
layout(location = 7) uniform sampler2D metalRoughnessTexture;
layout(location = 8) uniform sampler2D normalTexture;
layout(location = 9) uniform sampler2D occlusionTexture;
layout(location = 10) uniform sampler2D emissiveTexture;

layout(location = 11) uniform vec2 depthRange;

out float outDepth;

void main() {
	float alpha = texture(baseTexture, fragBaseTexCoord).a * material.baseColour.a * fragAlpha;
	if ((material.alphaMode == 2) && (alpha <= material.alphaCutoff)) discard;
	outDepth = (fragDepth - depthRange[0])/(depthRange[1] - depthRange[0]);
}
