#version 440 core

in vec4		ciPosition;
in vec3		ciNormal;

uniform mat4 ciModelMatrix;
uniform mat4 ciModelMatrixInverseTranspose;
uniform mat4 ciViewProjection;//view and projection together

out VertexData {
	vec3 worldPositionGeom;
	vec3 normalGeom;
} vVertexOut;

void main(){
	vVertexOut.worldPositionGeom = vec3(ciModelMatrix * vec4(ciPosition.xyz, 1));
	vVertexOut.normalGeom = normalize(mat3(ciModelMatrixInverseTranspose) * ciNormal);
	gl_Position = ciViewProjection * vec4(vVertexOut.worldPositionGeom, 1);
}