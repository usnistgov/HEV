varying vec4 Ce;		// view-space sphere center
varying vec4 Ee;		// eye-space eye location

uniform vec4 iris_Viewport;	// center and size of window
uniform float fatpointSize;	// size of spheres in view space

void main() {

    // regular transform so GL will know where to draw the sphere
    gl_Position = ftransform();

    gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;

    Ce = gl_Vertex;

    Ee = gl_ModelViewProjectionMatrixInverse*vec4(0.,0.,-1.,0.);

    gl_PointSize = fatpointSize / gl_Position.w * 
	iris_Viewport.z / length(gl_ModelViewProjectionMatrixInverse[0].xyz);

    // keep assigned point color
    gl_FrontColor = gl_Color;
}
