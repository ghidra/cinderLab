#version 420
#extension GL_ARB_shader_storage_buffer_object : require

uniform mat4 ciProjectionMatrix;
uniform mat4 ciViewMatrix;
uniform float uSceneScale;//for voxel scalling
uniform float uVoxelResolution;
//uniform mat4 ciModelView;
//uniform float spriteSize;

#include "dat.glsl"

layout( std430, binding = 1 )  buffer TriBuffer
{
    Geo triangles[];
};

out gl_PerVertex {
	vec4 gl_Position;
};

out block {
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
} Out;

void main()
{
	Geo tri = triangles[gl_VertexID];

	Out.Vp = (tri.P*uSceneScale)+0.5f;// * uVoxelResolution);//-0.5f;//the voxel position
	Out.N = tri.N;
	Out.uv = tri.uv.xy;
	Out.Cd = tri.Cd;
	Out.Cs = tri.Cs;
	Out.Ds = tri.Ds;
	Out.Rd = tri.Rd;
	Out.Rs = tri.Rs;
	Out.e = tri.e;
	Out.t = tri.t;
	Out.ri = tri.ri;

	//gl_Position = ciProjectionMatrix * (ciViewMatrix * vec4(tri.P,1));
	gl_Position = ciProjectionMatrix * (ciViewMatrix * vec4(tri.P,1));
}
