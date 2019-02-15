#version 450 core

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

layout(location = 0) uniform image3D texture3D;

in vec3 worldPositionFrag;

vec3 scaleAndBias(vec3 p) { return 0.5f * p + vec3(0.5f); }
bool isInsideCube(const vec3 p, float e) { return abs(p.x) < 1 + e && abs(p.y) < 1 + e && abs(p.z) < 1 + e; }

void main()
{
	vec3 color = vec3(0.0f);
	if(!isInsideCube(worldPositionFrag, 0)) return;

	vec3 voxel = scaleAndBias(worldPositionFrag);
	ivec3 dim = imageSize(texture3D);
	float alpha = pow(1 - material.transparency, 4); // For soft shadows to work better with transparent materials.
	vec4 res = alpha * vec4(vec3(color), 1);
    imageStore(texture3D, ivec3(dim * voxel), res);
}