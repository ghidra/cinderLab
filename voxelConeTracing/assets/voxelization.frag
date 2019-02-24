// Lit (diffuse) fragment voxelization shader.
// Author:	Fredrik Präntare <prantare@gmail.com> 
// Date:	11/26/2016
#version 420 core

struct Material {
	vec3 diffuseColor;
	vec3 specularColor;
	float diffuseReflectivity;
	float specularReflectivity;
	float emissivity;
	float transparency;
};

uniform Material material;
uniform vec3 cameraPosition;

layout(binding = 0, RGBA8) writeonly uniform image3D texture3D;

in VertexData {
	vec3 worldPositionFrag;
	vec3 normalFrag;
} vVertexIn;

float attenuate(float dist){ dist *= 1.1f; return 1.0f / (1.0  +  dist * dist); }

vec3 calculatePointLight(vec3 lightposition, vec3 lightcolor){
	const vec3 direction = normalize(lightposition - vVertexIn.worldPositionFrag);
	const float distanceToLight = distance(lightposition, vVertexIn.worldPositionFrag);
	const float attenuation = attenuate(distanceToLight);
	const float d = max(dot(normalize(vVertexIn.normalFrag), direction), 0.0f);
	return d  * attenuation * lightcolor;
};

vec3 scaleAndBias(vec3 p) { return 0.5f * p + vec3(0.5f); }
bool isInsideCube(const vec3 p, float e) { return abs(p.x) < 1 + e && abs(p.y) < 1 + e && abs(p.z) < 1 + e; }

void main()
{
	vec3 color = vec3(0.0f);
	if(!isInsideCube(vVertexIn.worldPositionFrag, 0)) return;

	// Calculate diffuse lighting fragment contribution.
	color += calculatePointLight( vec3(0.3,0.4,0.2),vec3(1,1,1) );
	vec3 spec = material.specularReflectivity * material.specularColor;
	vec3 diff = material.diffuseReflectivity * material.diffuseColor;
	color = (diff + spec) * color + clamp(material.emissivity, 0, 1) * material.diffuseColor;

	vec3 voxel = scaleAndBias(vVertexIn.worldPositionFrag);
	ivec3 dim =  ivec3(64,64,64);//imageSize(texture3D);
	float alpha = pow(1 - material.transparency, 4); // For soft shadows to work better with transparent materials.
	vec4 res = alpha * vec4(vec3(color), 1);

    //imageStore(texture3D, ivec3(dim * voxel), res);
    imageStore(texture3D, ivec3(dim * voxel), vec4(1,0,0,1));
}