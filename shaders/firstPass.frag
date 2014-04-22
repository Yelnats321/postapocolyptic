#version 330

layout(location = 0) out vec4 outColor;

uniform sampler2D tex;
uniform samplerCube cubemap;
uniform vec3 color;
uniform vec3 lightPos;
uniform float SHADOW_NEAR;
uniform float SHADOW_FAR;

in vec2 UV;
in vec3 position;

float VectorToDepthValue(vec3 Vec){
	vec3 AbsVec = abs(Vec);
	float LocalZcomp = max(AbsVec.x, max(AbsVec.y, AbsVec.z));

	//  float f = SHADOW_FAR;
	//  float n = SHADOW_NEAR;
	//this is an adjusted one, check if it works tho
	float NormZComp = (SHADOW_FAR+SHADOW_NEAR - 2*SHADOW_FAR*SHADOW_NEAR*LocalZcomp) / (SHADOW_FAR-SHADOW_NEAR);
//	float NormZComp = (SHADOW_FAR+SHADOW_NEAR) /   (SHADOW_FAR-SHADOW_NEAR) - (2*SHADOW_FAR*SHADOW_NEAR)/(SHADOW_FAR-SHADOW_NEAR)/LocalZcomp;
	return (NormZComp + 1.0) * 0.5;
}

float DepthToVectorValue(float depth){
	return (1.0/SHADOW_FAR)/(SHADOW_FAR/(SHADOW_FAR-SHADOW_NEAR)-depth);
}

void main(){
	outColor = vec4(color,0.3);
	vec3 ray = position-lightPos;
	float depth = DepthToVectorValue(texture(cubemap,ray).r);
	vec3 absRay = abs(ray);
	float distance = max(absRay.x, max(absRay.y, absRay.z));
	if(depth<=distance-0.09){
		discard;
	}
}
