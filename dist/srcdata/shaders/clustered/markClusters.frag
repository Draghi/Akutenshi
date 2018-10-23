#version 450

layout(early_fragment_tests) in;

// /////////// //
// // Input // //
// /////////// //

in vec2 fragTextureCoords[4];
in vec4 fragColour;

// ////////////// //
// // Uniforms // //
// ////////////// //

// Alpha-test
layout(binding = 0, packed) uniform Material {
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

layout(location = 1) uniform sampler2D baseTexture;
layout(location = 2) uniform sampler2D metalRoughnessTexture;
layout(location = 3) uniform sampler2D normalTexture;
layout(location = 4) uniform sampler2D occlusionTexture;
layout(location = 5) uniform sampler2D emissiveTexture;

// Cluster Info
layout(location = 6) uniform vec2 screenSize;
layout(location = 7) uniform vec2 depthRange;
layout(location = 8) uniform float vertFOV;
layout(location = 9) uniform vec3 clusterSize;

// Output Buffer
layout(binding = 1) buffer ClusterBuffer { uint cluster[]; };

void doClusterAssignment();

void main() {
	// Alpha-test fragment
	vec4 baseColour = fragColour * material.baseColour * texture(baseTexture, fragTextureCoords[material.baseCoordSet]);
	switch(material.alphaMode) {
		/* OPAQUE */ case 0: break;
		/* CUTOFF */ case 1: if (baseColour.a <= material.alphaCutoff) discard;
		/* TRANS  */ case 2: if (baseColour.a <=                  0.f) discard;
	};
	doClusterAssignment();
}

uint coordToIndex(uvec3 tileCoord) {
	uvec2 maxTiles = uvec2(screenSize/clusterSize.xy);
	return tileCoord.x + tileCoord.y*maxTiles.x + tileCoord.z*maxTiles.x*maxTiles.y;
}

void doClusterAssignment() {
	uvec2 tileCoord = uvec2(gl_FragCoord.xy/screenSize*clusterSize.xy);
	uint depthCoord = uint(log(gl_FragCoord.z/depthRange[0])/log(1+(clusterSize.z*2*tan(vertFOV))/screenSize.y));
	uint clusterIndex = coordToIndex(uvec3(tileCoord, depthCoord));
	atomicOr(cluster[clusterIndex], 1u);
}