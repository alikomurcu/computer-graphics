#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 Normal;
out vec3 Position;

uniform mat4 modelingMatrix;
uniform mat4 viewingMatrix;
uniform mat4 projectionMatrix;

void main()
{
	Normal = mat3(transpose(inverse(modelingMatrix))) * aNormal;
	Position = vec3(modelingMatrix * vec4(aPos, 1.0));
	gl_Position = projectionMatrix * viewingMatrix * modelingMatrix * vec4(aPos, 1.0);
}