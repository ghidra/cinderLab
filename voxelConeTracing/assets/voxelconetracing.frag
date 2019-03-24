//----------------------------------------------------------------------------------------------//
// A voxel cone tracing implementation for real-time global illumination,                       //
// refraction, specular, glossy and diffuse reflections, and soft shadows.                      //
// The implementation traces cones through a 3D texture which contains a                        //
// direct lit voxelized scene.                                                                  //
//                                                                                              //
// Inspired by "Interactive Indirect Illumination Using Voxel Cone Tracing" by Crassin et al.   //  
// (Cyril Crassin, Fabrice Neyret, Miguel Saintz, Simon Green and Elmar Eisemann)               //
// https://research.nvidia.com/sites/default/files/publications/GIVoxels-pg2011-authors.pdf     //
//                                                                                              //  
// Author:  Fredrik Präntare <prantare@gmail.com>                                               //
// Date:    11/26/2016                                                                          //
//                                                                                              // 
// Course project in TSBK03 (Techniques for Advanced Game Programming) at Linköping University. //
// ---------------------------------------------------------------------------------------------//
#version 420
#extension GL_ARB_shader_storage_buffer_object : require

#include "dat.glsl"

#define TSQRT2 2.828427
#define SQRT2 1.414213
#define ISQRT2 0.707106
// --------------------------------------
// Light (voxel) cone tracing settings.
// --------------------------------------
#define MIPMAP_HARDCAP 5.4f /* Too high mipmap levels => glitchiness, too low mipmap levels => sharpness. */
#define VOXEL_SIZE (1/64.0) /* Size of a voxel. 128x128x128 => 1/128 = 0.0078125. */
#define SHADOWS 1 /* Shadow cone tracing. */
#define DIFFUSE_INDIRECT_FACTOR 0.52f /* Just changes intensity of diffuse indirect lighting. */
// --------------------------------------
// Other lighting settings.
// --------------------------------------
#define SPECULAR_MODE 1 /* 0 == Blinn-Phong (halfway vector), 1 == reflection model. */
#define SPECULAR_FACTOR 4.0f /* Specular intensity tweaking factor. */
#define SPECULAR_POWER 65.0f /* Specular power in Blinn-Phong. */
#define DIRECT_LIGHT_INTENSITY 0.96f /* (direct) point light intensity factor. */
#define MAX_LIGHTS 1 /* Maximum number of lights supported. */

// Lighting attenuation factors. See the function "attenuate" (below) for more information.
#define DIST_FACTOR 1.1f /* Distance is multiplied by this when calculating attenuation. */
#define CONSTANT 1
#define LINEAR 0 /* Looks meh when using gamma correction. */
#define QUADRATIC 1

// Other settings.
#define GAMMA_CORRECTION 1 /* Whether to use gamma correction or not. */

// Basic point light.
struct PointLight {
    vec3 position;
    vec3 color;
};

uniform PointLight pointLights[MAX_LIGHTS];
uniform int numberOfLights; // Number of lights currently uploaded.

uniform vec3 uCameraPosition; // World campera position.
uniform float uVoxelResolution;
//uniform sampler3D texture3D; // Voxelization texture.

in block {
    vec3 Vp; //VoxelizedPosition
    vec3 N; //normal
    vec2 uv; //2 uv channels
    vec3 Cd; //color diffuse
    vec3 Cs; //color spec
    float Rd; //Reflectivity diffuse
    float Rs; //reflectivity specular
    float e; //emisivity
    float t; //tranparency
} In;

layout( std430, binding = 0 ) buffer VoxelBufferFull
{
    Voxel voxels[];
};
layout( std430, binding = 2 )  buffer VoxelBufferResized
{
    Voxel voxelsresized[];
};

layout( location = 0 ) out vec4 fragColor;

float fit(float v, float l1, float h1, float l2, float h2){return l2 + (v - l1) * (h2 - l2) / (h1 - l1);}

Voxel voxelMip(vec3 p, float lod)
{
    //this is a fake mipmap style look up of my voxel data in the ssbo
    //i get the weight per axis, then use that to linear blend the color of each axis
    
    Voxel mip;

    //get the lod blend value
    uint lod1 = uint(floor(lod));
    uint lod2 = uint(ceil(lod));
    float lodblend = lod-float(lod1);

    //scale the voxel position.
    float voxelDivisor = float(pow(2,lod1));//1,2,4,8
    //divisor of the scalled to voxel space.
    uint voxelRes = uint(uVoxelResolution)/uint(voxelDivisor);//this gives us the width that we are sitting in... ie, normal res is 128, lod1 is 64
    vec3 scaledPosition = (p*float(voxelRes))+((1.0f/voxelRes)*0.5f);
    //get my 3 axiscaledPositions weight
    
    //float xblend = fit( scaledPosition.x, floor(scaledPosition.x), floor(scaledPosition.x)+voxelDivisor, 0.0f, 1.0f );
    float xblend = fit( scaledPosition.x, floor(scaledPosition.x), ceil(scaledPosition.x), 0.0f, 1.0f );
    float yblend = fit( scaledPosition.y, floor(scaledPosition.y), ceil(scaledPosition.y), 0.0f, 1.0f );
    float zblend = fit( scaledPosition.z, floor(scaledPosition.z), ceil(scaledPosition.z) , 0.0f, 1.0f );
 
    //id offset
    uint ylowoff = voxelRes*voxelRes*uint(floor(scaledPosition.y));
    
    uint p0 = uint(floor(scaledPosition.x)) + (voxelRes*uint(floor(scaledPosition.z))) + ylowoff;//+uint(floor(scaledPosition.z));
    uint p1 = uint(ceil(scaledPosition.x)) + (voxelRes*uint(floor(scaledPosition.z))) + ylowoff;//+uint(floor(scaledPosition.z));
    uint p2 = uint(floor(scaledPosition.x)) + (voxelRes*uint(ceil(scaledPosition.z))) + ylowoff;
    uint p3 = uint(ceil(scaledPosition.x)) + (voxelRes*uint(ceil(scaledPosition.z))) + ylowoff;

    uint yhighoff = voxelRes*voxelRes*uint(ceil(scaledPosition.y));

    uint p4 = p0+yhighoff;
    uint p5 = p1+yhighoff;
    uint p6 = p2+yhighoff;
    uint p7 = p3+yhighoff;

    //weights based on axis blend
    float m0 = (1-xblend)*(1-yblend)*(1-zblend);
    float m1 = xblend*(1-yblend)*(1-zblend);
    float m2 = (1-xblend)*(1-yblend)*zblend;
    float m3 = xblend*(1-yblend)*zblend;
    float m4 = (1-xblend)*yblend*(1-zblend);
    float m5 = xblend*yblend*(1-zblend);
    float m6 = (1-xblend)*yblend*zblend;
    float m7 = xblend*yblend*zblend;

    //first set look up
    if(lod1>0)
    {
        uint r1 = uint( pow(uVoxelResolution/2,3)) * uint(min(max(lod1-1,0),1));//i1 == *0
        uint r2 = uint( pow( r1/2, 3 ) ) * uint( min(max(lod1-2, 0 ),1) );
        uint r3 = uint( pow( r2/2, 3 ) ) * uint( min(max(lod1-3, 0 ),1) );
        uint offset = r1+r2+r3;

        mip.Cd = voxelsresized[offset+p0].Cd * m0 + voxelsresized[offset+p1].Cd * m1 + voxelsresized[offset+p2].Cd * m2 + voxelsresized[offset+p3].Cd * m3 + voxelsresized[offset+p4].Cd * m4 + voxelsresized[offset+p5].Cd * m5 + voxelsresized[offset+p6].Cd * m6 + voxelsresized[offset+p7].Cd * m7;
        mip.Alpha = voxelsresized[offset+p0].Alpha * m0 + voxelsresized[offset+p1].Alpha * m1 + voxelsresized[offset+p2].Alpha * m2 + voxelsresized[offset+p3].Alpha * m3 + voxelsresized[offset+p4].Alpha * m4 + voxelsresized[offset+p5].Alpha * m5 + voxelsresized[offset+p6].Alpha * m6 + voxelsresized[offset+p7].Alpha * m7;
        
        //mip.Cd = voxelsresized[p0].Cd;
        //mip.Alpha = 1.0f;
    }
    else
    {
        //mip.Cd = voxelsresized[p7].Cd;
        mip.Cd = voxels[p0].Cd * m0 + voxels[p1].Cd * m1 + voxels[p2].Cd * m2 + voxels[p3].Cd * m3 + voxels[p4].Cd * m4 + voxels[p5].Cd * m5 + voxels[p6].Cd * m6 + voxels[p7].Cd * m7;
        mip.Alpha = voxels[p0].Alpha * m0 + voxels[p1].Alpha * m1 + voxels[p2].Alpha * m2 + voxels[p3].Alpha * m3 + voxels[p4].Alpha * m4 + voxels[p5].Alpha * m5 + voxels[p6].Alpha * m6 + voxels[p7].Alpha * m7;
    }
    
    return mip;
}

void main()
{
	//if the point has no normal... then we are likely not a triangle worth rendering
	if(length(In.Vp)<=0.001 )
	{
		discard;
	}
    Voxel v = voxelMip(In.Vp,0.0f);
    //fragColor = vec4( In.Cd.rgb, 1.0 );
    //fragColor = vec4( v.Cd+In.Cd.rgb*0.1f, 1.0 );
    fragColor = vec4( v.Cd, 1.0 );
}