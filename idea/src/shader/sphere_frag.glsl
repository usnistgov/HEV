#ifndef sphere_fragment_glsl
#define sphere_fragment_glsl

// compute ray/sphere intersections
//   fragCoord = pixel coordinate of ray
//   in Po = center of sphere in object space
//   in r = sphere radius in object space
//   in zrange = depth range for sphere (in -1..1 NDC)
//   out Fo0, No1 = object-space intersections
//   out No0, No1 = object-space normals
//   out d0, d1   = screen-space depths
void sphereTest(vec4 fragCoord, vec4 Po, float r, vec2 zrange,
		out vec4 Fo0, out vec3 No0, out float d0,
		out vec4 Fo1, out vec3 No1, out float d1)
{
    // avoid NVIDIA compiler bug with gl_ModelViewProjectionMatrixInverse
    mat4 MVP_i = gl_ModelViewMatrixInverse*gl_ProjectionMatrixInverse;

    // Ap = point along ray at front of sphere
    // Bp = point along ray at back of sphere
    vec2 vs = iris_Viewport.zw, vc = iris_Viewport.xy + .5*vs;
    vec2 fc = (fragCoord.xy-vc)*2./vs;
    vec4 Ap = vec4(fc, zrange.x, 1);
    vec4 Bp = vec4(fc, zrange.y, 1);
    vec4 Ao = MVP_i*Ap, Bo = MVP_i*Bp;  // A,B in object space

    // default points infinitely far down axis
    Fo0 = Fo1 = vec4(Bo.xyz*Ao.w - Ao.xyz*Bo.w, 0);
    No0 = No1 = vec3(0);
    d0 = d1 = 1./0.;

    // quadric matrix
    mat4 Q = mat4(Po.w*Po.w);
    Q[0][3] = Q[3][0] = -Po.x*Po.w;
    Q[1][3] = Q[3][1] = -Po.y*Po.w;
    Q[2][3] = Q[3][2] = -Po.z*Po.w;
    Q[3][3] = dot(Po.xyz,Po.xyz) - r*r*Q[3][3];

    ///// test sphere
    // plug ((1-t)*A + t*B) into quadric & solve for t
    float AA = dot(Ao,Q*Ao), AB = dot(Ao,Q*Bo), BB = dot(Bo,Q*Bo);
    float a = AA-2*AB+BB, b=AB-AA, c=AA;
    float d = sqrt(b*b - c*a);

    // avoid numeric problems
    if (isnan(d) || a<1e-6) return;

    // where did we hit?
    float h0 = (-d-b)/a; Fo0 = mix(Ao, Bo, h0);   No0 = (Q*Fo0).xyz;
    float h1 = ( d-b)/a; Fo1 = mix(Ao, Bo, h1);   No1 = (Q*Fo1).xyz;

    // project back to screen space to find fragment depth
    float fmn = gl_DepthRange.diff, fpn = gl_DepthRange.far+gl_DepthRange.near;
    vec2 drange = .5*fmn * zrange + .5*fpn;
    d0 = mix(drange.x, drange.y, h0);
    d1 = mix(drange.x, drange.y, h1);
}



// use with sphereTest to compute clipping for hollow sphere
//   plane = plane coefficients (e.g. from gl_ClipPlane[])
//   inout Fo0, No1 = object-space intersections
//   inout No0, No1 = object-space normals
//   inout d0, d1   = screen-space depths
void sphereClipHollow(
    vec4 plane, 
    inout vec4 Fe0, inout vec3 No0, inout float d0,
    inout vec4 Fe1, inout vec3 No1, inout float d1)
{
    // test front and back intersection against clip plane
    float c0 = dot(plane,Fe0);
    float c1 = dot(plane,Fe1);

    // both intersections clipped, nothing to see
    if ((d0<0 || c0<0) && (d1<0 || c1<0)) discard;

    // front clipped, ignore it by moving intersection behind viewer
    if (d0>0 && c0<0) {
	d0 = -1;
    }

    // back clipped, ignore it by moving intersection behind viewer
    if (d1>0 && c1<0) {
	d1 = -1;
    }
}


// use with sphereTest to compute clipping for solid sphere
//   plane = plane coefficients (e.g. from gl_ClipPlane[])
//   inout Fo0, No1 = object-space intersections
//   inout No0, No1 = object-space normals
//   inout d0, d1   = screen-space depths
void sphereClipSolid(
    vec4 plane, 
    inout vec4 Fe0, inout vec3 No0, inout float d0,
    inout vec4 Fe1, inout vec3 No1, inout float d1)
{
    // test front and back intersection against clip plane
    float c0 = dot(plane,Fe0);
    float c1 = dot(plane,Fe1);

    // both intersections clipped, nothing to see
    if ((d0<0 || c0<0) && (d1<0 || c1<0)) discard;

    // front clipped, compute plane intersection and normal
    if (d0>0 && c0<0) {
	// use normal to clip plane
	No0 = -(plane*gl_ModelViewMatrix).xyz;

	// compute intersection with clip plane
	vec4 Ee = -gl_ProjectionMatrixInverse[2];
	float ce = dot(plane,Ee);
	Fe0 = (Fe0*ce - Ee*c0)/(ce-c0);

	// compute screen depth at that point
	vec4 Fp0 = gl_ProjectionMatrix*Fe0;
	d0 = .5*(gl_DepthRange.diff*Fp0.z/Fp0.w + 
		 gl_DepthRange.far+gl_DepthRange.near);
    }

    // back clipped, compute plane intersection and normal
    if (d1>0 && c1<0) {
	// use normal to clip plane
	No1 = (plane*gl_ModelViewMatrix).xyz;

	// compute intersection with clip plane
	vec4 Ee = -gl_ProjectionMatrixInverse[2];
	float ce = dot(plane,Ee);
	Fe1 = (Fe1*ce - Ee*c1)/(ce-c1);

	// compute screen depth at that point
	vec4 Fp1 = gl_ProjectionMatrix*Fe1;
	d1 = .5*(gl_DepthRange.diff*Fp1.z/Fp1.w + 
		 gl_DepthRange.far+gl_DepthRange.near);
    }
}


// call from fragment shader to compute ray/sphere intersection w/o clipping
//   fragCoord = pixel coordinate of ray
//   in Po = center of sphere in object space
//   in r = sphere radius in object space
//   in zrange = depth range for sphere (in -1..1 NDC)
//   out Fe = eye-space intersection of ray and sphere
//   out Ne = eye-space normal
//   out face = -1 for intersection inside of sphere, +1 for outside
// return depth of intersection, infinity if no intersection
float sphereDraw(vec4 fragCoord, vec4 Po, float r, vec2 zrange,
		 out vec4 Fe, out vec3 Ne, out float face)
{
    // find both intersections
    vec4 Fo0, Fo1, Fo;
    vec3 No0, No1, No; 
    float d0, d1, d;
    sphereTest(fragCoord, Po, r, zrange, Fo0,No0,d0, Fo1,No1,d1);

    // find closest intersection
    Fo = vec4(0); No = vec3(0); face = 0; d = 1./0.;
    if (d0>=0) {
	Fo = Fo0; No = No0; face =  1; d = d0;
    }
    else if (d1>=0) {
	Fo = Fo1; No = No1; face = -1; d = d1;
    }

    // covert output to eye space
    Fe = gl_ModelViewMatrix * Fo;
    Ne = normalize(gl_NormalMatrix * No);
    return d;
}


////////////////////////////////////////////////////////////
// functions for computation of sphere color

// color of sphere in direction N, using cube map
vec4 sphereColorCubeMap(samplerCube tex, vec3 N) {
    return textureCube(tex, N);
}

// color of sphere in direction N, using sphere map
vec4 sphereColorSphMap(sampler2D tex, vec3 N) {
    vec3 NN = normalize(vec3(0,0,1) + N);
    return texture2D(tex, .5*NN.xy + .5);
}

// color of sphere in direction N, using spherical coordinats
vec4 sphereColorSphCoord(sampler2D tex, vec3 N) {
    vec2 scoord = vec2(.5+atan(N.x,N.z)/6.2831853,acos(-N.y)/3.1415926);
    return texture2D(tex, scoord);
}

#endif
