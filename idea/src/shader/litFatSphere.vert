// point vertex shader to find tight screen space bounds for a fat sphere

// Based Jim Blinn's Corner, How to Draw a Sphere: Part 1, IEEE CG&A,
// January 1995. 

uniform vec4 iris_Viewport;	// corner and size of window
uniform float fatpointSize=0.01;// size of spheres in view space

varying vec3 Po;		// true center of sphere

void main() {
    // regular transform stuff
    Po = gl_Vertex.xyz/gl_Vertex.w;
    gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_FrontColor = gl_Color;

    // inverse quadric matrix for sphere centered at origin
    //   q = plane*Q*transpose(plane) = dot(plane*Qs,plane)
    //   q<0 if plane intersects quadric, q=0 if tanget, q>0 no intersection
    mat4 Q = mat4(-1.); Q[3][3] = 1./fatpointSize/fatpointSize;

    // transform from sphere centered at origin to projection space
    mat4 Trans   = mat4(1); Trans[3].xyz =  Po;
    mat4 xform = gl_ModelViewProjectionMatrix*Trans;
    Q = xform*Q*transpose(xform);

    // find x & y bounds as solutions for tangent planes:
    //   Px = (1,0,0,-x); Py = (0,1,0,-y)
    // e.g. for x: Px*Q*transpose(Px) gives quadratic equation
    //   a*x^2 - 2*b*x + c : a=Q[3][3], b=Q[0][3], c=Q[0][0]
    float a = Q[3].w;		// 'a' same for x&y
    vec3 b = Q[3].xyz;		// note -2 factored out of 'b'
    vec3 c = vec3(Q[0][0],Q[1][1],Q[2][2]);  // 'c' is Q diagonal
    vec3 d = b*b-a*c;		// discriminant of quadratic equation

    // frustum culling: no intersection with edge and outside it
    // let system cull by leaving point location alone & setting size to 0
    // sideEdges:(left,bottom,right,top); nearFar:(near,far)
    vec4 sideEdges = vec4(a + 2.*b.xy + c.xy, a - 2.*b.xy + c.xy);
    vec2 nearFar = vec2(a + 2.*b.z + c.z, a - 2.*b.z + c.z);
    if ((gl_Position.x < -gl_Position.w  &&  sideEdges[0]>0.) ||
	(gl_Position.y < -gl_Position.w  &&  sideEdges[1]>0.) ||
	(gl_Position.z < -gl_Position.w  &&  nearFar[0]>0.) ||
	(gl_Position.x > gl_Position.w  &&  sideEdges[2]>0.) ||
	(gl_Position.y > gl_Position.w  &&  sideEdges[3]>0.) ||
	(gl_Position.z > gl_Position.w  &&  nearFar[1]>0.) ) {

	gl_PointSize=0.; return;
    }

    // find screen space boundinb box (xmin,ymin,xmax,ymax)
    // solutions for projection to ellispe (d>0) or parabola (d=0)
    // problem guaranteed to intersect view frustum, so fixed below
    vec4 ssbb = vec4(b.xy-sqrt(d.xy), b.xy+sqrt(d.xy))/a;

    // clamp to screen bounds if intersects edges
    if (sideEdges[0] < 0.) ssbb[0] = -1.;
    if (sideEdges[1] < 0.) ssbb[1] = -1.;
    if (sideEdges[2] < 0.) ssbb[2] = 1.;
    if (sideEdges[3] < 0.) ssbb[3] = 1.;

    // cull if nothing visible
    if (ssbb[0]>=ssbb[2] || ssbb[1]>=ssbb[3]) {gl_PointSize=0.; return;}

    // turn bounds into size
    vec2 vs = iris_Viewport.zw, size;
    size.y = max((ssbb[2]-ssbb[0])*vs.y/vs.x, ssbb[3]-ssbb[1]);
    size.x = size.y*vs.x/vs.y;

    // adjust center to move overdrawn pixels off screen if we can
    vec2 e = .5*(ssbb.xy+ssbb.zw-size);
    if (e.x < -1.) ssbb[0] = ssbb[2]-size.x;
    if (e.y < -1.) ssbb[1] = ssbb[3]-size.y;
    e = .5*(ssbb.xy+ssbb.zw+size);
    if (e.x > 1.) ssbb[2] = ssbb[0]+size.x;
    if (e.y > 1.) ssbb[3] = ssbb[1]+size.y;

    // position actual vertex in middle of bounds
    gl_Position.xy = .5*(ssbb.xy+ssbb.zw)*gl_Position.w;
    gl_PointSize = .5*size.x*vs.x;
}
