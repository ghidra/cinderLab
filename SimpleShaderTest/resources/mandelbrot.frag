#version 420

#include "noiselib.glsl"

// Sets the maximum number of iterations per pixel.
// Note: anything above 256 is a waste of energy,
//       because of the limited floating point precision.
#define Iterations 6

uniform sampler2D uTex0;
//uniform vec2      uCenter;
uniform float     uAspectRatio;// = 1.33333;
//uniform float     uScale = 1.0;
uniform float uFreq = 1.0f;
uniform float uTime;
uniform float uPhase = 0.1f;
uniform float uTestArray[Iterations];

in vec2 vertTexCoord0;

out vec4 fragColor;

void main()
{
	vec2 cuv = ( vertTexCoord0 - vec2(0.5) ) * vec2(uAspectRatio,1.0);//make it "square" by strecting out x
	float outside = 1.0;
	for (int i=0; i<Iterations; i++)
	{
		float nx = SimplexPerlin3D( vec3( 0.0, 0.0, (float(i)*uFreq) + (uTime*uPhase) ) );
		float ny = SimplexPerlin3D( vec3( 30.0, 192.4, ( float(i)*uFreq ) + (uTime*uPhase) ) );
		vec2 center = vec2(nx,ny)*0.5;
		float d = length( cuv - center );
		outside *= min(d*12.0*uTestArray[i],1.0f);
	}
		

	

	// Lookup color from LUT.
	//vec3 color = texture( uTex0, texCoord ).rgb;

	// Output color.
	fragColor = vec4( vec3(outside), 1.0 );
}