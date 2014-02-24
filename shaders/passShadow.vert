#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 position;

uniform mat4 MVP;
void main(){
	gl_Position = MVP * vec4(position,1);
}
