#version 120
// point vertex shader to find tight screen space bounds for a fat sphere

// Based Jim Blinn's Corner, How to Draw a Sphere: Part 1, IEEE CG&A,
// January 1995. 

uniform vec4 iris_Viewport;	// corner and size of window
uniform float fatpointSize;	// size of spheres in view space

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
    mat4 Q = mat4(-1.); Q[3][3] = 1./(fatpointSize*fatpointSize);

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
    vec4 ssbb = clamp(vec4(b.xy-sqrt(d.xy), b.xy+sqrt(d.xy))/a, -1.,1.);

    // cull if nothing visible
    if (ssbb[0]>=ssbb[2] || ssbb[1]>=ssbb[3]) {gl_PointSize=0.; return;}

    // turn bounds into size in pixels then back to screen space
    float pxSize = max((ssbb[2]-ssbb[0])*iris_Viewport.z,
		       (ssbb[3]-ssbb[1])*iris_Viewport.w);
    vec2 ssSize = pxSize/iris_Viewport.zw;

    // adjust center to move overdrawn pixels off screen if we can
    vec2 e = .5*(ssbb.xy+ssbb.zw-ssSize);
    if (e.x < -1.) ssbb[0] = ssbb[2]-ssSize.x;
    if (e.y < -1.) ssbb[1] = ssbb[3]-ssSize.y;
    e = .5*(ssbb.xy+ssbb.zw+ssSize);
    if (e.x > 1.) ssbb[2] = ssbb[0]+ssSize.x;
    if (e.y > 1.) ssbb[3] = ssbb[1]+ssSize.y;

    // position actual vertex in middle of bounds
    gl_Position.xy = .5*(ssbb.xy+ssbb.zw)*gl_Position.w;
    gl_PointSize = .5*pxSize;
}
