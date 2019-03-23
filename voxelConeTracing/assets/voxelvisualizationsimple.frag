#version 420
#extension GL_ARB_shader_storage_buffer_object : enable
//#extension GL_ARB_compute_variable_group_size : enable
//uniform sampler3D uVoxels;

#include "dat.glsl"

uniform float      uVoxelResolution;
uniform uint uOffset;
uniform vec2      uResolution;

/*struct Voxel
{
  //vec3 P;  //position
  vec3 N;  //Normal
  vec3 Cd; //color
};*/

layout( std430, binding = 0 ) buffer Vox
{
    Voxel voxels[];
};

in vec2 vertTexCoord0;

out vec4 fragColor;

void main()
{
	float split = sqrt( uVoxelResolution );
	//vec3 spos = vec3(vertTexCoord0,uVoxelResolution/2.0f);

	//get the pixel id
	vec2 id = vertTexCoord0*(uResolution-vec2(1.0));
	vec2 subuv = mod(id,vec2(uVoxelResolution))/vec2(uVoxelResolution);

	float subid = subuv.x+(subuv.y*uVoxelResolution);
	float substep = subid/(uVoxelResolution);

	///get the big blocks
	vec2 layer = floor(id/vec2(uVoxelResolution));
	if(layer.x>=split || layer.y>=split)
		discard;
	vec2 layer_z = mod(layer,vec2(split))/vec2(split);
	float sublayer = layer_z.x+(layer_z.y*split);
	float substeplayer = sublayer/(split);

	//fragColor = vec4( voxel.xyz+vec3(subuv,substeplayer), 1.0 );
	fragColor = vec4( vec3(subuv,substeplayer), 1.0 );
	//fragColor = vec4( vec3(0.0,0.0,substeplayer), 1.0 );

	//vec4 voxel = texture(uVoxels,vec3(subuv,substeplayer));
	//fragColor = vec4( voxel.rgb + (vec3(subuv,substeplayer)*0.1), 1.0 );;
	//fragColor = vec4(vec3(1,0,0),1.0);

	vec3 vox = round( vec3(subuv,substeplayer)*(uVoxelResolution-1));
	int ind = int( vox.x+(vox.y*uVoxelResolution)+(vox.z*uVoxelResolution*uVoxelResolution) );
	//if( length(voxels[ind+uOffset].N)<=0.001) discard;
	//if( length(voxels[ind+uOffset].Alpha)<=0.001) discard;
	fragColor = vec4(voxels[ind+uOffset].Cd,1.0f);
	///simple version
	//fragColor = vec4(voxels[ int(id.x+(id.y*uResolution.x)) ].Cd,1.0f);
}