#version 460 core

uniform mat4 modelingMatrix;
uniform mat4 viewingMatrix;
uniform mat4 projectionMatrix;

layout(location=0) in vec3 inVertex;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec3 inTexCoord;

out vec3 fragCoord;
out vec3 texCoord;

void main(void)
{
//    texCoord = vec3(modelingMatrix * vec4(inTexture.st, sliceID / (numSlices -1), 1));
    texCoord = vec3(modelingMatrix * vec4(inTexCoord.st, 1, 1));
    fragCoord = vec3(viewingMatrix * modelingMatrix * vec4(inVertex, 1));
    gl_Position = viewingMatrix * modelingMatrix * vec4(inVertex,1);
    //    gl_Position = vec4(vec2(inVertex), 0, 1);
}
