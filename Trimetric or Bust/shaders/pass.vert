#version 330 core

layout(location = 0)in vec3 position;
out vec2 UV;
void main(){
	UV = position.xy;
	gl_Position = vec4(position*2-1, 1);
}
