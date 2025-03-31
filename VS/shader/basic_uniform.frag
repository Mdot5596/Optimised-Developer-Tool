#version 460

in vec2 TexCoords;
in vec3 Position;    // Fragment position in camera space
in vec3 Normal;
in vec3 Vec;

layout(location = 0) out vec4 FragColor;
layout(binding = 0) uniform samplerCube SkyBoxTex; //maybe change to 3
layout(binding = 1) uniform sampler2D TextureMap;
layout(binding = 2) uniform sampler2D SecondTextureMap;

uniform float texScale;
uniform float mixFactor;
uniform bool UseSecondTexture;
uniform bool IsSkybox;

uniform mat4 ViewMatrix;  

// Light struct
uniform struct SpotLightInfo {
    vec3 Position;    // World space position
    vec3 L;
    vec3 La;
    vec3 Direction;   // World space direction
    float Exponent;
    float Cutoff;
} Spot;

// Material struct
uniform struct MaterialInfo {
    vec3 Kd;
    vec3 Ka;
    vec3 Ks;
    float Shininess;
} Material;

// Fog struct
uniform struct FogInfo {
    float MaxDist;
    float MinDist;
    vec3 Color;
} Fog;

vec3 blinnPhongSpot(vec3 position, vec3 n)
{
    // Transform spotlight position and direction to camera space
    vec3 spotPosCamera = (ViewMatrix * vec4(Spot.Position, 1.0)).xyz;
    vec3 spotDirCamera = normalize((ViewMatrix * vec4(Spot.Direction, 0.0)).xyz);

    // Ambient component
    vec3 ambient = Spot.La * Material.Ka;

    // Light direction
    vec3 s = normalize(spotPosCamera - position);

    float cosAng = dot(-s, spotDirCamera); 
    float angle = acos(cosAng);
    float spotScale = 0.0;

    if (angle < Spot.Cutoff) {
        spotScale = pow(cosAng, Spot.Exponent);

        float sDotN = max(dot(s, n), 0.0);
        vec3 diffuse = Material.Kd * sDotN;

        vec3 spec = vec3(0.0);
        if (sDotN > 0.0) {
            vec3 v = normalize(-position);
            vec3 h = normalize(v + s);
            spec = Material.Ks * pow(max(dot(h, n), 0.0), Material.Shininess);
        }

        return ambient + spotScale * Spot.L * (diffuse + spec);
    }

    return ambient;
}

void main() 
{
    if (IsSkybox) {
        vec3 texColor = texture(SkyBoxTex, normalize(Vec)).rgb;
        FragColor = vec4(texColor, 1.0);
        return;
    }

    float dist = length(Position);
    float fogFactor = (Fog.MaxDist - dist) / (Fog.MaxDist - Fog.MinDist);
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    vec4 texColor1 = texture(TextureMap, TexCoords * texScale);

    vec4 finalTex = texColor1;
    if (UseSecondTexture) {
        vec4 texColor2 = texture(SecondTextureMap, TexCoords * texScale);
        finalTex = mix(texColor1, texColor2, 0.5);
    }

    vec3 shadeColor = blinnPhongSpot(Position, normalize(Normal));

    vec3 finalColor = mix(Fog.Color, shadeColor * finalTex.rgb, fogFactor);

    FragColor = vec4(finalColor, finalTex.a);
}