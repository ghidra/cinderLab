// Simple non-conservative voxelization.
// Implementation inspired by Cheng-Tso Lin: 
// https://github.com/otaku690/SparseVoxelOctree/blob/master/WIN/SVO/shader/voxelize.geom.glsl.
// Author:	Fredrik Präntare <prantare@gmail.com>
// Date:	11/26/2016
#version 440 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VertexData {
	vec3 worldPositionGeom;
	vec3 normalGeom;
} vVertexIn[];

out VertexData {
	vec3 worldPositionFrag;
	vec3 normalFrag;
} vVertexOut;

void main(){
	const vec3 p1 = vVertexIn[1].worldPositionGeom - vVertexIn[0].worldPositionGeom;
	const vec3 p2 = vVertexIn[2].worldPositionGeom - vVertexIn[0].worldPositionGeom;
	const vec3 p = abs(cross(p1, p2)); 
	for(uint i = 0; i < 3; ++i){
		vVertexOut.worldPositionFrag = vVertexIn[i].worldPositionGeom;
		vVertexOut.normalFrag = vVertexIn[i].normalGeom;
		if(p.z > p.x && p.z > p.y){
			gl_Position = vec4(vVertexOut.worldPositionFrag.x, vVertexOut.worldPositionFrag.y, 0, 1);
		} else if (p.x > p.y && p.x > p.z){
			gl_Position = vec4(vVertexOut.worldPositionFrag.y, vVertexOut.worldPositionFrag.z, 0, 1);
		} else {
			gl_Position = vec4(vVertexOut.worldPositionFrag.x, vVertexOut.worldPositionFrag.z, 0, 1);
		}
		EmitVertex();
	}
    EndPrimitive();
}