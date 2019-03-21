struct Voxel
{
  //ec3 P;  //position
  //vec3 N;//normal
  vec3 Cd; //color
  float Alpha;//voxel transparency
};
struct Geo{
	vec3 P; //position
  vec3 N; //normal
  vec4 uv; //2 uv channels
  vec3 Cd; //color diffuse
  vec3 Cs; //color spec
  float Rd; //Reflectivity diffuse
  float Rs; //reflectivity specular
  float e; //emisivity
  float t; //tranparency
  //vec4 lit;///lighting result
};