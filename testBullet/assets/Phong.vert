#version 150

uniform mat4	ciModelViewProjection;
uniform mat4	ciModelView;
uniform mat4	ciModelMatrix;
uniform mat3	ciNormalMatrix;

in vec4		ciPosition;
in vec3		ciNormal;
in vec4		ciColor;

out VertexData {
	vec4 position;
	vec4 worldposition;
	vec3 normal;
	vec4 color;
} vVertexOut;

void main(void) {
	vVertexOut.position = ciModelView * ciPosition;
	vVertexOut.worldposition = ciModelMatrix * ciPosition;
	vVertexOut.normal = ciNormalMatrix * ciNormal;
	vVertexOut.color = ciColor;
	gl_Position = ciModelViewProjection * ciPosition;
}