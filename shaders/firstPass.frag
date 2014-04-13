#version 330

layout(location = 0) out vec4 outColor;

uniform sampler2D tex;
uniform samplerCubeShadow cubemap;
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
    float NormZComp = (SHADOW_FAR+SHADOW_NEAR) / (SHADOW_FAR-SHADOW_NEAR) - (2*SHADOW_FAR*SHADOW_NEAR)/(SHADOW_FAR-SHADOW_NEAR)/LocalZcomp;
    return (NormZComp + 1.0) * 0.5;
}

void main(){
	outColor = vec4(color,0.3);
	float VtoD = VectorToDepthValue(lightPos-position);
	if(texture(cubemap, vec4( position - lightPos ,VtoD-0.001))!= 1){
		discard;
	}
}
