#version 420

#include "noiselib.glsl"

//uniform sampler2D uTex0;
uniform vec2      uResolution;
uniform float     uAspectRatio;// = 1.33333;


const float boxSize = 100.0;
const float borderSize = 20.0;

in vec2 vertTexCoord0;

out vec4 fragColor;

void main()
{
	vec2 fragposition = vertTexCoord0 * uResolution;
	vec2 fraggrid = mod(fragposition,vec2(boxSize)) /vec2( boxSize) ;//this is the box uv

	// float borderratio = borderSize/boxSize;//this is the box uv

	// float borderx1 = 1.0-ceil(fraggrid.x-(borderratio*0.5));
	// float borderx2 = floor(fraggrid.x+(borderratio*0.5));
	// float borderx = max(borderx1,borderx2);
	// float bordery1 = 1.0-ceil(fraggrid.y-(borderratio*0.5));
	// float bordery2 = floor(fraggrid.y+(borderratio*0.5));
	// float bordery = max(bordery1,bordery2);
	// float border = max(borderx,bordery);
	vec2 borderxy = pow( 1.0-((sin((fraggrid)*vec2(3.14))+1.0)*0.5),vec2(2.8));
	float border = min(pow(max(borderxy.x,borderxy.y),0.2)*1.5,0.2);
 
	vec2 fragdiv = floor(fragposition/vec2(boxSize));
	float n_small = SimplexPerlin3D(vec3((fragposition+(fragdiv*19.0))*0.01,0.0));
	float n_large = SimplexPerlin3D(vec3(fragposition*0.005,0.0));
	float n_xlarge = SimplexPerlin3D(vec3(fragposition*vec2(1.0,uAspectRatio)*0.01,0.0));

	float ceinter_distance = distance((vertTexCoord0-0.5)+(n_xlarge*0.1),vec2(0.0));
	float inside = 1.0-min(ceil(ceinter_distance-0.3),1.0);
	float insidesmooth = min(pow(1.0-min(ceinter_distance-0.2,1.0),108.2),1.0);

	float n = mix(n_small,n_large,border)*insidesmooth; 
	//boxSize

	float nscaled = 1.0-pow((sin(n *160.4)+1.0)*0.5,4.2);

	// Lookup color from LUT.
	//vec3 color = texture( uTex0, texCoord ).rgb;

	// Output color.
	fragColor = vec4( vec3(n_small), 1.0 );
}