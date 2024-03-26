#version 330 core

in vec2 PositionOS;
in vec3 Color;
in mat4 ModelMatrix; //Instanced matrix

uniform mat4 ViewMatrix;
uniform mat4 ProjMatrix;

out vec3 color;

void main()
{
    gl_Position = ProjMatrix * ViewMatrix * ModelMatrix * vec4(PositionOS, 0.0, 1.0);
    color = Color;
}