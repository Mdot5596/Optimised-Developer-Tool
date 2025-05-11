#version 460

in vec3 Position;
in vec3 Normal;

layout (binding=0) uniform sampler2D Texture0;

uniform int Pass;
uniform float Weight[5];


layout (location = 0) out vec4 FragColor;

uniform struct LightInfo {
   vec4 Position;
   vec3 La;
   vec3 L;
} Light; 


uniform struct MaterialInfo{
   vec3 Kd;
   vec3 Ka;
   vec3 Ks;
   float Shininess;
}Material;


vec3 blinnPhong ( vec3 position, vec3 n)
{
    vec3 diffuse=vec3(0), spec=vec3(0);
  //  vec3 texColor=texture(RenderTex, TexCoord).rgb;
    vec3 ambient=Light.La*Material.Ka;
    vec3 s=normalize(Light.Position.xyz-position);
    float sDotN=max(dot(s,n),0.0);
    diffuse=Material.Kd*sDotN;
    if (sDotN>0.0)
     {
    vec3 v=normalize(-position.xyz);
    vec3 h=normalize(v+s);
    spec=Material.Ks*pow(max(dot(h,n),0.0),Material.Shininess);
      }
    return ambient+(diffuse+spec)*Light.L;
}

vec4 pass1()
{
 return vec4(blinnPhong(Position, normalize(Normal)), 1.0);
}

vec4 pass2()
{
ivec2 pix=ivec2(gl_FragCoord.xy);
vec4 sum=texelFetch(Texture0, pix, 0)*Weight[0];
sum+=texelFetchOffset(Texture0, pix,0, ivec2(0,1))*Weight[1];
sum+=texelFetchOffset(Texture0, pix,0, ivec2(0,-1))*Weight[1];
sum+=texelFetchOffset(Texture0, pix,0, ivec2(0,2))*Weight[2];
sum+=texelFetchOffset(Texture0, pix,0, ivec2(0,-2))*Weight[2];
sum+=texelFetchOffset(Texture0, pix,0, ivec2(0,3))*Weight[3];
sum+=texelFetchOffset(Texture0, pix,0, ivec2(0,-3))*Weight[3];
sum+=texelFetchOffset(Texture0, pix,0, ivec2(0,4))*Weight[4];
sum+=texelFetchOffset(Texture0, pix,0, ivec2(0,-4))*Weight[4];
return sum;
}

vec4 pass3()
{
ivec2 pix=ivec2(gl_FragCoord.xy);
vec4 sum=texelFetch(Texture0, pix, 0)*Weight[0];
sum+=texelFetchOffset(Texture0, pix,0, ivec2(1,0))*Weight[1];
sum+=texelFetchOffset(Texture0, pix,0, ivec2(-1,0))*Weight[1];
sum+=texelFetchOffset(Texture0, pix,0, ivec2(2,0))*Weight[2];
sum+=texelFetchOffset(Texture0, pix,0, ivec2(-2,0))*Weight[2];
sum+=texelFetchOffset(Texture0, pix,0, ivec2(3,0))*Weight[3];
sum+=texelFetchOffset(Texture0, pix,0, ivec2(-3,0))*Weight[3];
sum+=texelFetchOffset(Texture0, pix,0, ivec2(4,0))*Weight[4];
sum+=texelFetchOffset(Texture0, pix,0, ivec2(-4,0))*Weight[4];
return sum;
}

void main()
{
   if (Pass==1) FragColor=pass1();
  else if (Pass==2) FragColor=pass2();
  else if (Pass==3) FragColor=pass3();
}