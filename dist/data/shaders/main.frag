#version 450

in vec3 fPos;
in vec3 fNorm;
in vec2 fTex;
in vec3 fCol;
in mat3 fMatTBN;

layout(location = 3) uniform sampler2D uTex;
layout(location = 4) uniform vec3 uVecCameraPos;
layout(location = 5) uniform sampler2D uNorm;
layout(location = 6) uniform sampler2D uSpec;


out vec4 fragColour;

void main() {
	vec3 wLightDir = normalize(vec3(0.66, 1.50, -0.75));
	vec3 lightDir = normalize(fMatTBN * wLightDir);
	
	vec3 norm = normalize(texture(uNorm, fTex).rgb*2 - 1);
	
	float diffuseFactor = clamp(dot(norm, lightDir), 0, 1);
	vec3 texColour = texture(uTex, fTex).rgb;
	
	float specularFactor = 0;
	vec3 viewDir = normalize(fMatTBN * (uVecCameraPos - fPos));
	vec3 halfAngle = normalize(lightDir + viewDir);
	specularFactor = clamp(dot(norm, halfAngle), 0, 1);
	vec3 specColour = texture(uSpec, fTex).rgb;
		
	float ambientFactor = 0.1;
	fragColour = vec4(
		ambientFactor*texColour/*fCol*/ +
		diffuseFactor*texColour/*fCol*/ + 
		pow(specularFactor,25)*specColour, 1.0);
}
