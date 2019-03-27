#version 420

#include "dat.glsl"

in block {
    vec3 Vp; //VoxelizedPosition
    vec3 N; //normal
    vec2 uv; //2 uv channels
    vec3 Cd; //color diffuse
    vec3 Cs; //color spec
    float Ds;
    float Rd; //Reflectivity diffuse
    float Rs; //reflectivity specular
    float e; //emisivity
    float t; //tranparency
    float ri; //refraction index
} In;

layout( location = 0 ) out vec4 fragColor;

void main()
{
	if(length(In.N)<=0.001 )
	{
		discard;
	}

    fragColor = vec4( In.N.rgb, 1.0 );
}
