// 1D transfer function for voltrace volume renderer
// For volume rendering with the gradient-based lighting model

// 1D transfer function mapping scalar values to color and opacity
uniform sampler2D volumeLUT;

// specular color and power
uniform vec4 sref = vec4(.04,.04,.04, 32);

// lookup transfer function with lighting on opaque voxels
//    p = position of this sample
//    prev = previous position
vec4 transfer(vec3 p, inout vec4 prev) {

    vec4 val = voldata(p);
    vec4 xfer = texture2D(volumeLUT, val.rr);


    // this version only does lighting for opaque regions in the
    // transfer function
    if (xfer.a >= 1.) {
	// vector from current point in volume to light & viewer
	vec4 Lp = gl_ModelViewMatrixInverse*gl_LightSource[0].position;
	vec4 Vp = -gl_ModelViewProjectionMatrixInverse[2];
	vec3 L = normalize(Lp.xyz-p*Lp.w);
	vec3 V = normalize(Vp.xyz-p*Vp.w);
	vec3 H = normalize(L+V);

	// compute normal from gradient: could preprocess to store in
	// volume (say val.xyz=normal, val.w=actual scalar value) for
	// now, do an incredibly inefficient extra six volume accesses.
	vec2 d = vec2(1./voxelSize.x,0.);
	vec3 N = normalize(vec3(voldata(p-d.xyy).r - voldata(p+d.xyy).r,
				voldata(p-d.yxy).r - voldata(p+d.yxy).r,
				voldata(p-d.yyx).r - voldata(p+d.yyx).r));

	// cap at clipping planes
	vec4 near = vec4(0.,0.,1.,1.)*gl_ModelViewProjectionMatrix;
	if (dot(near,prev)<=0) N = normalize(-near.xyz);
	for(int i=0; i<gl_MaxClipPlanes; ++i) {
	    vec4 plane = gl_ClipPlane[i]*gl_ModelViewMatrix;
	    if (dot(plane, prev)<0) N = normalize(plane.xyz);
	}

	// compute diffuse & normalized specular terms
	// for specular normalization, see e.g. 
	// http://renderwonk.com/publications/s2010-shading-course/
	float diffuse = max(0.,dot(N,L));
	float specular = pow(max(0,dot(N,H)), sref.a)*(sref.a+2)*.5;

	xfer.rgb = diffuse*(xfer.rgb + sref.rgb*specular);
    }

    prev = vec4(p,1.);

    return xfer;
}
