varying vec4 Ce;		// view-space sphere center
varying vec4 Ee;		// eye-space eye location

varying float sides;		// number of sides (from gl_Normal.x)
varying float size;		// size of glyph (fatpointSize*gl_Normal.y)
varying float eyespace;		// 0 means use eye space, !0 means object space (from gl_Normal.z)

uniform vec4 iris_Viewport;	// center and size of window
uniform float fatpointSize;	// size of spheres in view space

void main() {
    // regular transform so GL will know where to draw the sphere
    gl_Position = ftransform();

    sides = gl_Normal.x;
    size = fatpointSize * gl_Normal.y;
    eyespace = gl_Normal.z; 

    // sphere center in eye space for fragment computation and clipping
    Ce = gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;

    // parallel & perspective-safe comptuation of eye location
    Ee = gl_ProjectionMatrixInverse*vec4(0.,0.,-1.,0.);

    // scale point size from object to eye space
    if (eyespace != 0.0)
	size *= length(gl_ModelViewMatrix[0].xyz);

    // compute point size in pixels based on scaling along x direction
    gl_PointSize = size * length(gl_ProjectionMatrix[0].xyz)
	* iris_Viewport.z / gl_Position.w;

    // keep assigned point color
    gl_FrontColor = gl_Color;
}
