#version 460

in vec2 TexCoords;
in vec3 Position;    
in vec3 Normal;
in vec3 Vec;

layout(location = 0) out vec4 FragColor;
layout(binding = 0) uniform samplerCube SkyBoxTex;
layout(binding = 1) uniform sampler2D TextureMap;
layout(binding = 2) uniform sampler2D SecondTextureMap;

uniform float texScale;
uniform float mixFactor;
uniform bool UseSecondTexture;
uniform bool IsSkybox;

uniform struct MaterialInfo {
    vec3 Kd;
    vec3 Ka;
    vec3 Ks;
    float Shininess;
} Material;

void main() 
{
    if (IsSkybox) {
        vec3 texColor = texture(SkyBoxTex, normalize(Vec)).rgb;
        FragColor = vec4(texColor, 1.0);
        return;
    }

    vec4 texColor1 = texture(TextureMap, TexCoords * texScale);
    vec4 finalTex = texColor1;

    if (UseSecondTexture) {
        vec4 texColor2 = texture(SecondTextureMap, TexCoords * texScale);
        finalTex = mix(texColor1, texColor2, 0.5);
    }


    vec3 ambient = Material.Ka;
    vec3 finalColor = ambient * finalTex.rgb;

    FragColor = vec4(finalColor, finalTex.a);
}
