#version 330

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 uv;

uniform mat4 MVP;
uniform mat4 M;

out vec2 UV;
out vec3 position;

void main()
{
	position =(M*vec4(inPosition,1.0)).xyz;
	gl_Position = MVP * vec4(inPosition, 1);
	UV = uv;
}

