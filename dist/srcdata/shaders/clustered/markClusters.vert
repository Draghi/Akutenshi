#version 450

// /////////// //
// // Input // //
// /////////// //

layout(location = 0) in vec3 vertexPosition;
layout(location = 6) in vec2 vertexTextureCoord[4];
layout(location = 10) in vec4 vertexColour;

// ////////////// //
// // Uniforms // //
// ////////////// //

layout(location = 0) uniform mat4 matModelWorldViewProj;

// //////////// //
// // Output // //
// //////////// //

out vec2 fragTextureCoords[4];
out vec4 fragColour;

void main() {
	for(uint i = 0; i < 4; i++) fragTextureCoords[i] = vertexTextureCoord[i];
	fragColour = vertexColour;
	gl_Position = matModelWorldViewProj * vec4(vertexPosition, 1);
}

