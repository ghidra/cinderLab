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
#define MIPMAP_HARDCAP 3.0f /* Too high mipmap levels => glitchiness, too low mipmap levels => sharpness. */
//#define VOXEL_SIZE (1/64.0) /* Size of a voxel. 128x128x128 => 1/128 = 0.0078125. */
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
uniform float uSceneScale;
//uniform sampler3D texture3D; // Voxelization texture.

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

layout( std430, binding = 0 ) buffer VoxelBufferFull
{
    Voxel voxels[];
};
layout( std430, binding = 2 )  buffer VoxelBufferResized
{
    Voxel voxelsresized[];
};

layout( location = 0 ) out vec4 fragColor;

///////////////////////////

vec3 normal = normalize(In.N); 
float MAX_DISTANCE = 2.0f;//distance(vec3(abs(worldPositionFrag)), vec3(-1));
float VOXEL_SIZE = 1.0f/uVoxelResolution;

// Returns an attenuation factor given a distance.
float attenuate(float dist){ dist *= DIST_FACTOR; return 1.0f / (1 + dist * dist); }

// Returns a vector that is orthogonal to u.
vec3 orthogonal(vec3 u){
    u = normalize(u);
    vec3 v = vec3(0.99146, 0.11664, 0.05832); // Pick any normalized vector.
    return abs(dot(u, v)) > 0.99999f ? cross(u, vec3(0, 1, 0)) : cross(u, v);
}

// Scales and bias a given vector (i.e. from [-1, 1] to [0, 1]).
vec3 scaleAndBias(const vec3 p) { return 0.5f * p + vec3(0.5f); }

// Returns true if the point p is inside the unity cube. 
bool isInsideCube(const vec3 p, float e) { return abs(p.x) < 1 + e && abs(p.y) < 1 + e && abs(p.z) < 1 + e; }


///////////////////////////

float fit(float v, float l1, float h1, float l2, float h2){return l2 + (v - l1) * (h2 - l2) / (h1 - l1);}

vec4 voxelMip(vec3 p, float lod)
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
    
    //return mip;
    return vec4(mip.Cd,mip.Alpha);
}

///////////////////////////

// Returns a soft shadow blend by using shadow cone tracing.
// Uses 2 samples per step, so it's pretty expensive.
float traceShadowCone(vec3 from, vec3 direction, float targetDistance){
    from += normal * 0.05f; // Removes artifacts but makes self shadowing for dense meshes meh.

    float acc = 0;

    float dist = 3 * VOXEL_SIZE;
    // I'm using a pretty big margin here since I use an emissive light ball with a pretty big radius in my demo scenes.
    const float STOP = targetDistance - 16 * VOXEL_SIZE;

    while(dist < STOP && acc < 1){  
        vec3 c = from + dist * direction;
        if(!isInsideCube(c, 0)) break;
        c = scaleAndBias(c);
        float l = pow(dist, 2); // Experimenting with inverse square falloff for shadows.
        
        float s1 = 0.062 * voxelMip(c,1 + 0.75 * l).a;
        float s2 = 0.135 * voxelMip(c,4.5 * l).a;
        //float s1 = 0.062 * textureLod(texture3D, c, 1 + 0.75 * l).a;
        //float s2 = 0.135 * textureLod(texture3D, c, 4.5 * l).a;
        
        float s = s1 + s2;
        acc += (1 - acc) * s;
        dist += 0.9 * VOXEL_SIZE * (1 + 0.05 * l);
    }
    return 1 - pow(smoothstep(0, 1, acc * 1.4), 1.0 / 1.4);
}

// Traces a diffuse voxel cone.
vec3 traceDiffuseVoxelCone(const vec3 from, vec3 direction){
    direction = normalize(direction);
    
    const float CONE_SPREAD = 0.325;

    vec4 acc = vec4(0.0f);

    // Controls bleeding from close surfaces.
    // Low values look rather bad if using shadow cone tracing.
    // Might be a better choice to use shadow maps and lower this value.
    float dist = 0.1953125;

    // Trace.
    while(dist < SQRT2 && acc.a < 1){
        vec3 c = from + dist * direction;
        c = scaleAndBias(from + dist * direction);
        float l = (1 + CONE_SPREAD * dist / VOXEL_SIZE);
        float level = log2(l);
        float ll = (level + 1) * (level + 1);
        
        vec4 voxel = voxelMip(c, min(MIPMAP_HARDCAP, level));
        //vec4 voxel = textureLod(texture3D, c, min(MIPMAP_HARDCAP, level));
        
        acc += 0.075 * ll * voxel * pow(1 - voxel.a, 2);
        dist += ll * VOXEL_SIZE * 2;
    }
    return pow(acc.rgb * 2.0, vec3(1.5));
}

// Traces a specular voxel cone.
vec3 traceSpecularVoxelCone(vec3 from, vec3 direction){
    direction = normalize(direction);

    const float OFFSET = 8 * VOXEL_SIZE;
    const float STEP = VOXEL_SIZE;

    from += OFFSET * normal;
    
    vec4 acc = vec4(0.0f);
    float dist = OFFSET;

    // Trace.
    while(dist < MAX_DISTANCE && acc.a < 1){ 
        vec3 c = from + dist * direction;
        if(!isInsideCube(c, 0)) break;
        c = scaleAndBias(c); 
        
        float level = 0.1 * In.Ds * log2(1 + dist / VOXEL_SIZE);
        vec4 voxel = voxelMip(c, min(level, MIPMAP_HARDCAP));
        //vec4 voxel = textureLod(texture3D, c, min(level, MIPMAP_HARDCAP));
        float f = 1 - acc.a;
        acc.rgb += 0.25 * (1 + In.Ds) * voxel.rgb * voxel.a * f;
        acc.a += 0.25 * voxel.a * f;
        dist += STEP * (1.0f + 0.125f * level);
    }
    return 1.0 * pow(In.Ds + 1.0, 0.8) * acc.rgb;
}

// Calculates indirect diffuse light using voxel cone tracing.
// The current implementation uses 9 cones. I think 5 cones should be enough, but it might generate
// more aliasing and bad blur.
vec3 indirectDiffuseLight(){
    const float ANGLE_MIX = 0.5f; // Angle mix (1.0f => orthogonal direction, 0.0f => direction of normal).

    const float w[3] = {1.0, 1.0, 1.0}; // Cone weights.

    // Find a base for the side cones with the normal as one of its base vectors.
    const vec3 ortho = normalize(orthogonal(normal));
    const vec3 ortho2 = normalize(cross(ortho, normal));

    // Find base vectors for the corner cones too.
    const vec3 corner = 0.5f * (ortho + ortho2);
    const vec3 corner2 = 0.5f * (ortho - ortho2);

    // Find start position of trace (start with a bit of offset).
    const vec3 N_OFFSET = normal * (1 + 4 * ISQRT2) * VOXEL_SIZE;
    const vec3 C_ORIGIN = In.Vp + N_OFFSET;

    // Accumulate indirect diffuse light.
    vec3 acc = vec3(0);

    // We offset forward in normal direction, and backward in cone direction.
    // Backward in cone direction improves GI, and forward direction removes
    // artifacts.
    const float CONE_OFFSET = -0.01;

    // Trace front cone
    acc += w[0] * traceDiffuseVoxelCone(C_ORIGIN + CONE_OFFSET * normal, normal);

    // Trace 4 side cones.
    const vec3 s1 = mix(normal, ortho, ANGLE_MIX);
    const vec3 s2 = mix(normal, -ortho, ANGLE_MIX);
    const vec3 s3 = mix(normal, ortho2, ANGLE_MIX);
    const vec3 s4 = mix(normal, -ortho2, ANGLE_MIX);

    acc += w[1] * traceDiffuseVoxelCone(C_ORIGIN + CONE_OFFSET * ortho, s1);
    acc += w[1] * traceDiffuseVoxelCone(C_ORIGIN - CONE_OFFSET * ortho, s2);
    acc += w[1] * traceDiffuseVoxelCone(C_ORIGIN + CONE_OFFSET * ortho2, s3);
    acc += w[1] * traceDiffuseVoxelCone(C_ORIGIN - CONE_OFFSET * ortho2, s4);

    // Trace 4 corner cones.
    const vec3 c1 = mix(normal, corner, ANGLE_MIX);
    const vec3 c2 = mix(normal, -corner, ANGLE_MIX);
    const vec3 c3 = mix(normal, corner2, ANGLE_MIX);
    const vec3 c4 = mix(normal, -corner2, ANGLE_MIX);

    acc += w[2] * traceDiffuseVoxelCone(C_ORIGIN + CONE_OFFSET * corner, c1);
    acc += w[2] * traceDiffuseVoxelCone(C_ORIGIN - CONE_OFFSET * corner, c2);
    acc += w[2] * traceDiffuseVoxelCone(C_ORIGIN + CONE_OFFSET * corner2, c3);
    acc += w[2] * traceDiffuseVoxelCone(C_ORIGIN - CONE_OFFSET * corner2, c4);

    // Return result.
    return DIFFUSE_INDIRECT_FACTOR * In.Rd * acc * (In.Cd + vec3(0.001f));
}


// Calculates indirect specular light using voxel cone tracing.
vec3 indirectSpecularLight(vec3 viewDirection){
    const vec3 reflection = normalize(reflect(viewDirection, normal));
    return In.Rs * In.Cs * traceSpecularVoxelCone(In.Vp, reflection);
}

// Calculates refractive light using voxel cone tracing.
vec3 indirectRefractiveLight(vec3 viewDirection){
    const vec3 refraction = refract(viewDirection, normal, 1.0 / In.ri);
    const vec3 cmix = mix(In.Cs, 0.5 * (In.Cs + vec3(1)), In.t);
    return cmix * traceSpecularVoxelCone(In.Vp, refraction);
}

// Calculates diffuse and specular direct light for a given point light.  
// Uses shadow cone tracing for soft shadows.
//vec3 calculateDirectLight(const PointLight light, const vec3 viewDirection){
//    vec3 lightDirection = light.position - worldPositionFrag;
vec3 calculateDirectLight(vec3 lightposition, vec3 lightcolor, const vec3 viewDirection){
    vec3 lightDirection = ((lightposition*uSceneScale)+vec3(0.5)) - In.Vp;
    const float distanceToLight = length(lightDirection);
    lightDirection = lightDirection / distanceToLight;
    const float lightAngle = dot(normal, lightDirection);
    
    // --------------------
    // Diffuse lighting.
    // --------------------
    float diffuseAngle = max(lightAngle, 0.0f); // Lambertian.  
    
    // --------------------
    // Specular lighting.
    // --------------------
#if (SPECULAR_MODE == 0) /* Blinn-Phong. */
    const vec3 halfwayVector = normalize(lightDirection + viewDirection);
    float specularAngle = max(dot(normal, halfwayVector), 0.0f);
#endif
    
#if (SPECULAR_MODE == 1) /* Perfect reflection. */
    const vec3 reflection = normalize(reflect(viewDirection, normal));
    float specularAngle = max(0, dot(reflection, lightDirection));
#endif

    float refractiveAngle = 0;
    if(In.t > 0.01){
        vec3 refraction = refract(viewDirection, normal, 1.0 / In.ri);
        refractiveAngle = max(0, In.t * dot(refraction, lightDirection));
    }

    // --------------------
    // Shadows.
    // --------------------
    float shadowBlend = 1;
#if (SHADOWS == 1)
    if(diffuseAngle * (1.0f - In.t) > 0)
        shadowBlend = traceShadowCone(In.Vp, lightDirection, distanceToLight);
#endif

    // --------------------
    // Add it all together.
    // --------------------
    diffuseAngle = min(shadowBlend, diffuseAngle);
    specularAngle = min(shadowBlend, max(specularAngle, refractiveAngle));
    const float df = 1.0f / (1.0f + 0.25f * In.Ds); // Diffusion factor.
    const float specular = SPECULAR_FACTOR * pow(specularAngle, df * SPECULAR_POWER);
    const float diffuse = diffuseAngle * (1.0f - In.t);

    const vec3 diff = In.Rd * In.Cd * diffuse;
    const vec3 spec = In.Rs * In.Cs * specular;
    const vec3 total = lightcolor * (diff + spec);
    return attenuate(distanceToLight) * total;
};
// Sums up all direct light from point lights (both diffuse and specular).
vec3 directLight(vec3 viewDirection){
    vec3 direct = vec3(0.0f);
    const uint maxLights = min(numberOfLights, MAX_LIGHTS);
    //for(uint i = 0; i < maxLights; ++i) direct += calculateDirectLight(pointLights[i], viewDirection);
    for(uint i = 0; i < maxLights; ++i) direct += calculateDirectLight(vec3(0.3,0.4,0.2),vec3(1,1,1), viewDirection);

    direct *= DIRECT_LIGHT_INTENSITY;
    return direct;
}

//////////////////////////

void main()
{
	//if the point has no normal... then we are likely not a triangle worth rendering
	if(length(In.Vp)<=0.001 )
	{
		discard;
	}
    //vec4 v = voxelMip(In.Vp,0.5f);
    ////fragColor = vec4( In.Cd.rgb, 1.0 );
    ////fragColor = vec4( v.Cd+In.Cd.rgb*0.1f, 1.0 );
    //fragColor = vec4( v.rgb, 1.0 );


    //////////////////////////

    vec4 color = vec4(0, 0, 0, 1);
    const vec3 viewDirection = normalize(In.Vp - ((uCameraPosition*uSceneScale)+vec3(0.5)) );

    // Indirect diffuse light.
    if(In.Rd * (1.0f - In.t) > 0.01f) 
        color.rgb += indirectDiffuseLight();

    // Indirect specular light (glossy reflections).
    if(In.Rs * (1.0f - In.t) > 0.01f) 
        color.rgb += indirectSpecularLight(viewDirection);

    // Emissivity.
    color.rgb += In.e * In.Cd;

    // Transparency
    if(In.t > 0.01f)
        color.rgb = mix(color.rgb, indirectRefractiveLight(viewDirection), In.t);

    // Direct light.
    //if(settings.directLight)
    color.rgb += directLight(viewDirection);

    fragColor = color;
}