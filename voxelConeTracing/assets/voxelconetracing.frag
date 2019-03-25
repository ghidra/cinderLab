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
    float voxelDivisor1 = float(pow(2,lod1));//1,2,4,8
    //divisor of the scalled to voxel space.
    uint voxelRes1 = uint(uVoxelResolution/voxelDivisor1);//this gives us the width that we are sitting in... ie, normal res is 128, lod1 is 64
    vec3 scaledPosition1 = (p*float(voxelRes1))+((1.0f/voxelRes1)*0.5f);
    //get my 3 axiscaledPositions weight
    
    //float xblend = fit( scaledPosition.x, floor(scaledPosition.x), floor(scaledPosition.x)+voxelDivisor, 0.0f, 1.0f );
    float xblend1 = fit( scaledPosition1.x, floor(scaledPosition1.x), ceil(scaledPosition1.x), 0.0f, 1.0f );
    float yblend1 = fit( scaledPosition1.y, floor(scaledPosition1.y), ceil(scaledPosition1.y), 0.0f, 1.0f );
    float zblend1 = fit( scaledPosition1.z, floor(scaledPosition1.z), ceil(scaledPosition1.z) , 0.0f, 1.0f );
 
    //id offset
    uint ylowoff1 = voxelRes1*voxelRes1*uint(floor(scaledPosition1.y));
    
    uint p10 = uint(floor(scaledPosition1.x)) + (voxelRes1*uint(floor(scaledPosition1.z))) + ylowoff1;//+uint(floor(scaledPosition.z));
    uint p11 = uint(ceil(scaledPosition1.x)) + (voxelRes1*uint(floor(scaledPosition1.z))) + ylowoff1;//+uint(floor(scaledPosition.z));
    uint p12 = uint(floor(scaledPosition1.x)) + (voxelRes1*uint(ceil(scaledPosition1.z))) + ylowoff1;
    uint p13 = uint(ceil(scaledPosition1.x)) + (voxelRes1*uint(ceil(scaledPosition1.z))) + ylowoff1;

    uint yhighoff1 = voxelRes1*voxelRes1*uint(ceil(scaledPosition1.y));

    uint p14 = p10+yhighoff1;
    uint p15 = p11+yhighoff1;
    uint p16 = p12+yhighoff1;
    uint p17 = p13+yhighoff1;

    //weights based on axis blend
    float m10 = (1-xblend1)*(1-yblend1)*(1-zblend1);
    float m11 = xblend1*(1-yblend1)*(1-zblend1);
    float m12 = (1-xblend1)*(1-yblend1)*zblend1;
    float m13 = xblend1*(1-yblend1)*zblend1;
    float m14 = (1-xblend1)*yblend1*(1-zblend1);
    float m15 = xblend1*yblend1*(1-zblend1);
    float m16 = (1-xblend1)*yblend1*zblend1;
    float m17 = xblend1*yblend1*zblend1;

    vec3 rounded = round(p*float(voxelRes1));
    uint pr = uint(rounded.x) + (voxelRes1*uint(rounded.z)) + (voxelRes1*voxelRes1*uint(rounded.y));//+uint(floor(scaledPosition.z));

    //first set look up
    if(lod1>0)
    {
        //uint r1 = uint( pow(uVoxelResolution/2,3)) * uint(min(max(lod1-1,0),1));//i1 == *0
        //uint r2 = uint( pow( r1/2, 3 ) ) * uint( min(max(lod1-2, 0 ),1) );
        //uint r3 = uint( pow( r2/2, 3 ) ) * uint( min(max(lod1-3, 0 ),1) );
        //uint offset = r1+r2+r3;

        //mip.Cd = voxelsresized[offset+p0].Cd * m0 + voxelsresized[offset+p1].Cd * m1 + voxelsresized[offset+p2].Cd * m2 + voxelsresized[offset+p3].Cd * m3 + voxelsresized[offset+p4].Cd * m4 + voxelsresized[offset+p5].Cd * m5 + voxelsresized[offset+p6].Cd * m6 + voxelsresized[offset+p7].Cd * m7;
        //mip.Alpha = voxelsresized[offset+p0].Alpha * m0 + voxelsresized[offset+p1].Alpha * m1 + voxelsresized[offset+p2].Alpha * m2 + voxelsresized[offset+p3].Alpha * m3 + voxelsresized[offset+p4].Alpha * m4 + voxelsresized[offset+p5].Alpha * m5 + voxelsresized[offset+p6].Alpha * m6 + voxelsresized[offset+p7].Alpha * m7;

        mip.Cd = voxelsresized[pr].Cd;
        mip.Alpha = 1.0f;
    }
    else
    {
        //vec3 rounded = round(p*float(voxelRes));
        //uint pr = uint(rounded.x) + (voxelRes*uint(rounded.z)) + (voxelRes*voxelRes*uint(rounded.y));//+uint(floor(scaledPosition.z));

        mip.Cd = voxels[pr].Cd;//vec3(float(pr)/float(voxelRes*voxelRes*voxelRes));
        mip.Alpha = 1.0;
        //mip.Cd = voxels[p0].Cd * m0 + voxels[p1].Cd * m1 + voxels[p2].Cd * m2 + voxels[p3].Cd * m3 + voxels[p4].Cd * m4 + voxels[p5].Cd * m5 + voxels[p6].Cd * m6 + voxels[p7].Cd * m7;
        //mip.Alpha = voxels[p0].Alpha * m0 + voxels[p1].Alpha * m1 + voxels[p2].Alpha * m2 + voxels[p3].Alpha * m3 + voxels[p4].Alpha * m4 + voxels[p5].Alpha * m5 + voxels[p6].Alpha * m6 + voxels[p7].Alpha * m7;
    }

    ///////////////////

    float voxelDivisor2 = float(pow(2,lod2));//1,2,4,8
    uint voxelRes2 = uint(uVoxelResolution/voxelDivisor2);//this gives us the width that we are sitting in... ie, normal res is 128, lod1 is 64
    vec3 scaledPosition2 = (p*float(voxelRes2))+((1.0f/voxelRes2)*0.5f);

    vec3 rounded2 = round(p*float(voxelRes2));
    uint pr2 = uint(rounded2.x) + (voxelRes2*uint(rounded2.z)) + (voxelRes2*voxelRes2*uint(rounded2.y));//+uint(floor(scaledPosition.z));

    if(lod>0.0)
    {
        mip.Cd = mix(mip.Cd,voxelsresized[pr2].Cd,lodblend);
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
    Voxel v = voxelMip(In.Vp,0.5f);
    //fragColor = vec4( In.Cd.rgb, 1.0 );
    //fragColor = vec4( v.Cd+In.Cd.rgb*0.1f, 1.0 );
    fragColor = vec4( v.Cd, 1.0 );
}