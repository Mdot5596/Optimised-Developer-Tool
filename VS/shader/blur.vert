#version 460

layout(location = 0) in vec3 VertexPosition; // Position of the vertex
layout(location = 1) in vec3 VertexNormal;   // Normal vector of the vertex

out vec3 Position;
out vec3 Normal;

uniform mat4 ModelViewMatrix;    // Model-View matrix
uniform mat3 NormalMatrix;       // Normal transformation matrix
uniform mat4 MVP;                // Model-View-Projection matrix


void main() 
{
   Normal = normalize(NormalMatrix*VertexNormal);
   Position =(ModelViewMatrix*vec4(VertexPosition,1.0)).xyz;

   gl_Position = MVP*vec4(VertexPosition,1.0);

}