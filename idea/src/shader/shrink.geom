// shrink primitive vertices toward the centroid of the primitive

// declare GLSL version and enable geometry shader extension
#version 120
#extension GL_EXT_geometry_shader4 : enable

uniform float shrinkFactor;

void main() {
    // find centroid
    vec4 c = vec4(0.);
    for (int i=0; i<gl_VerticesIn; ++i)
	c += gl_PositionIn[i];
    c /= float(gl_VerticesIn);

    // move vertices toward centroid
    for (int i=0; i<gl_VerticesIn; ++i) {
	gl_Position = mix(gl_PositionIn[i], c, shrinkFactor);
	gl_FrontColor = gl_FrontColorIn[i];
	gl_BackColor  = gl_BackColorIn[i];
	for(int j=0; j<gl_MaxTextureCoords; ++j)
	    gl_TexCoord[j] = gl_TexCoordIn[i][j];
	EmitVertex();
    }
    EndPrimitive();
}
