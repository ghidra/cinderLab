#if !defined ( NUM_MATERIALS )
#define NUM_MATERIALS 1
#endif

struct Material
{
	vec3 diffuseColor;
	vec3 specularColor;
	float diffuseReflectivity;
	float specularReflectivity;
	float emissivity;
	float transparency;
	uint	pad0;
	uint	pad1;
};

layout (std140) uniform Materials
{
	Material uMaterials[ NUM_MATERIALS ];
};