#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec3 ourColor;
out vec2 TexCoord;
uniform mat4 modelingMatrix;
uniform mat4 viewingMatrix;
uniform mat4 projectionMatrix;
void main()
{
    ourColor = aColor;
    TexCoord = vec2(aTexCoord.x, aTexCoord.y);
    gl_Position = projectionMatrix * viewingMatrix * modelingMatrix * vec4(aPos, 1);
}

