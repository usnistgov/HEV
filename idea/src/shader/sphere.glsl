#ifndef sphere_glsl
#define sphere_glsl

// Functions to support drawing spheres as fat points
// requires:
//   uniform vec4 iris_Viewport
// plus these GL state uniforms:
//   gl_ModelViewMatrix
//   gl_ModelViewProjectionMatrix
//   gl_ModelViewMatrixInverse
//   gl_ProjectionMatrixInverse
//   gl_NormalMatrix
//   gl_DepthRange

// Based Jim Blinn's Corner, How to Draw a Sphere: Part 1, IEEE CG&A,
// January 1995. 

// call from geometry or vertex shader
// computes screen space bounding box of sphere
//   Po = sphere center in object space
//   r = sphere radius in object space
//   out ssbb = screen space bounding box (left, top, right, bottom)
//   out zrange = range of z values (near, far)
// returns true if sphere is clipped, false if not
bool sphereBBox(vec4 Po, float r, 
		out vec4 ssbb, out vec2 zrange)
{
    ssbb = vec4(-1,-1,1,1); zrange = vec2(-1,1);

    // avoid NVIDIA compiler bug with gl_ModelViewProjectionMatrixInverse
    mat4 MVP_i = gl_ModelViewMatrixInverse*gl_ProjectionMatrixInverse;

    // quadric and inverse quadric matrix for sphere centered at origin
    //   q = plane*Qi*transpose(plane) = dot(plane*Qi,plane)
    //   q<0 if plane intersects quadric, q=0 if tanget, q>0 no intersection
    mat4 Qi = mat4(-r*r); Qi[3][3] = 1.;

    // normal to view plane; sphere is bounded by +/-r in vn direction
    //  mid-Z plane = 0,0,1,0 in projectionn space
    //  = gl_ModelViewProjectionMatrix * vec4(0,0,1,0) in model space
    //  = transpose(gl_ModelViewProjectionMatrix)[2]
    vec3 vn = normalize(transpose(gl_ModelViewProjectionMatrix)[2].xyz);

    // transform z tangent planes to projection space to find near & far z
    //   equation for point P on the plane tangent to the far end of the sphere
    //     dot(vn,P.xyz/P.w) - dot(vn,Po.xyz/Po.w) = r
    //     dot(vn, P.xyz)*Po.w - dot(vn,Po.xyz)*P.w - r*Po.w*P.w= 0
    //     dot(vec4(vn*Po.w, -dot(vn,Po.xyz)-r*Po.w), P) = 0
    // so vec4(vn*Po.w, -dot(vn,Po.xyz)-r*Po.w) defines the far tangent plane
    //  & vec4(vn*Po.w, -dot(vn,Po.xyz)+r*Po.w) defines the near tangent plane
    // transform by MVP_i to get from model back to projection space
    vec4 z0 = vec4(vn*Po.w, -dot(vn,Po.xyz) + r*Po.w)*MVP_i;
    vec4 z1 = vec4(vn*Po.w, -dot(vn,Po.xyz) - r*Po.w)*MVP_i;
    zrange.x = max(-z0.w, -z0.z) / z0.z;
    zrange.y = min(-z1.w,  z1.z) / z1.z;

    // outside of near or far planes
    if (zrange.x > 1 || zrange.y < -1) return true;


    // transform from sphere centered at origin to projection space
    mat4 Trans = mat4(Po.w); Trans[3].xyz =  Po.xyz;
    mat4 xform = gl_ModelViewProjectionMatrix*Trans;
    Qi = xform*Qi*transpose(xform);


    // find xy bounds as solutions for tangent planes:
    //   Px = (1,0,0,-x); Py = (0,1,0,-y)
    // e.g. for x: Px*Qi*transpose(Px) gives quadratic equation
    //   a*x^2 - 2*b*x + c : a=Qi[3][3], b=Qi[0][3], c=Qi[0][0]
    float a = Qi[3][3];			// 'a' same for x&y
    vec2 b = Qi[3].xy;			// note -2 factored out of 'b'
    vec2 c = vec2(Qi[0][0],Qi[1][1]);  // 'c' is Q diagonal
    vec2 d = sqrt(b*b-a*c);	// discriminant of quadratic equation


    // min and max solutions for x & y
    ssbb = vec4(b-d, b+d);
    ssbb = clamp(ssbb, -a,a)/a;
    if (any(equal(ssbb, vec4(1,1,-1,-1)))) return true; // outside view
    ssbb = mix(ssbb, vec4(-1,-1,1,1), isnan(d.xyxy)); // clamp if sqrt was NaN
    return false;
}


// compute point center and size for sphere vertex shader
//   Po = sphere center in object space
//   r = sphere radius in object space
//   out zrange = range of z values (near, far)
//   out spherePos = screen space position for GL_POINT
//   out sphereSize = size for GL_POINT
// returns true if sphere is clipped, false if not
bool sphereSize(vec4 Po, float r,
		out vec2 zrange, out vec4 spherePos, out float sphereSize)
{
    // screen space bounds
    vec4 ssbb; 

    // default position will be culled
    spherePos = vec4(0,0,-1,0);
    sphereSize = 0;

    // compute screen-space bounds
    if (sphereBBox(Po, r, ssbb, zrange))
	return true;

    // turn bound into pixels, find the biggest, then convert back
    float pxSize = max((ssbb.z-ssbb.x)*iris_Viewport.z,
		       (ssbb.w-ssbb.y)*iris_Viewport.w);
    vec2 ssSize = pxSize/iris_Viewport.zw;

    // adjust center to move overdrawn pixels off screen if we can
    vec2 e = 0.5*(ssbb.xy + ssbb.zw - ssSize);
    if (e.x < -1.) ssbb.x = ssbb.z - ssSize.x;
    if (e.y < -1.) ssbb.y = ssbb.w - ssSize.y;
    e = 0.5*(ssbb.xy + ssbb.zw + ssSize);
    if (e.x > 1.) ssbb.z = ssbb.x + ssSize.x;
    if (e.y > 1.) ssbb.w = ssbb.y + ssSize.y;

    // position actual vertex at far side of sphere, in middle of bounds
    spherePos = vec4(.5*(ssbb.xy + ssbb.zw), zrange.y, 1.);
    sphereSize = .5*pxSize;
    return false;
}

#endif
