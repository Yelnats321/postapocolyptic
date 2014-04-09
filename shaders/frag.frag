#version 330

layout(location = 0) out vec4 outColor;

uniform sampler2D tex;
uniform samplerCubeShadow cubemap;
uniform vec3 color;
uniform vec3 lightPos;
uniform float alpha;

in vec2 UV;
in vec3 position;

float VectorToDepthValue(vec3 Vec){
    vec3 AbsVec = abs(Vec);
    float LocalZcomp = max(AbsVec.x, max(AbsVec.y, AbsVec.z));

    const float f = 5.0;
    const float n = 0.1;
    float NormZComp = (f+n) / (f-n) - (2*f*n)/(f-n)/LocalZcomp;
    return (NormZComp + 1.0) * 0.5;
}

void main(){
	outColor = vec4(color,1);
	float VtoD = VectorToDepthValue(lightPos-position);
	if(texture(cubemap, vec4( position - lightPos ,VtoD-0.001))!= 1){
		outColor.a = alpha;
		if(alpha == 0) discard;
	}
}
