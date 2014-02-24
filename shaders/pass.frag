#version 330 core

in vec2 UV;
layout(location = 0)out vec3 color;

uniform samplerCube map;
void main(){
	color = vec3(texture(map,vec3(UV,1)).x);
}
