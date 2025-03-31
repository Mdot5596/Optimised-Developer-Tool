#version 460

layout(location = 0) in vec3 VertexPosition; // Position of the vertex
layout(location = 1) in vec3 VertexNormal;   // Normal vector of the vertex
layout(location = 2) in vec2 VertexTexCoord;  // Position of the vertex

out vec2 TexCoords;                           // Pass texture coordinates to fragment shader
out vec3 Position;
out vec3 Normal;
out vec3 Vec;

uniform mat4 ModelViewMatrix;    // Model-View matrix
uniform mat3 NormalMatrix;       // Normal transformation matrix
uniform mat4 ProjectionMatrix;
uniform mat4 MVP;                // Model-View-Projection matrix


void main() 
{
   Normal = normalize(NormalMatrix*VertexNormal);
   Position =(ModelViewMatrix*vec4(VertexPosition,1.0)).xyz;

   TexCoords = VertexTexCoord;

  // Vec = VertexPosition; 
    Vec = (vec4(VertexPosition, 0.0)).xyz; // for static skybox in world space


   gl_Position = MVP*vec4(VertexPosition,1.0);

}