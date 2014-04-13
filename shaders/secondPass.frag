#version 330

layout(location = 0) out vec4 outColor;

uniform sampler2D tex;
uniform vec3 color;

in vec2 UV;
in vec3 position;

void main(){
	outColor = vec4(color*0.3,1);
}
