#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
out vec3 outNormal;
uniform mat4 model;
uniform mat4 modelView;
uniform mat4 projection;
void main()
{
	outNormal =  mat3(transpose(model)) * normal;
	gl_Position = projection * modelView * model * vec4(pos, 1.0);
}
