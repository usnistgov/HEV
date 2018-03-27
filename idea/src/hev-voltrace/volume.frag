// volume rendering ray caster, supporting clipping and embedded objects

#include "uniform.glsl"

// maximum steps to take along any ray
#define MAXSTEPS 1000

in vec4 Eo;		// eye location in object space
in vec4 Vo;		// position in object space
in vec4 DebugColor;	// color each plane for debugging

out vec4 FragColor;

//////////////////////////////
// get volume data
// should define voldata() returning a vec4 data value
//
// vec4 voldata(vec4 p)
//    p = position of this sample
#include VOLDATA

//////////////////////////////
// transfer function
// should define transfer() returning sample color & opacity
//
// vec4 transfer(vec3 pos, inout vec4 pVal)
//    pos = position of this sample
//    pVal = previous sample value
//
// transfer should call voldata as necessary to sample the volume
//
// pVal is only present to provide persistent state for transfer()
// transfer() is responsible for updating it if it uses it.
#include TRANSFER

//////////////////////////////
// integration function
// should define integration() returning color & opacity of a step
//
// vec4 integration(vec3 pos, float step, 
//                  inout vec3 pPos, inout vec4 pVal, inout vec4 pCol)
//    pos = position of this sample
//    step = size of the integration step
//    pPos, pVal, pCol = previous position, value and color
//
// integration() should call transfer if necessary
//
// pPos, pVal & pCol are only present to provide persistent state for
// integration(). integration() is responsible for updating them if it
// uses them.
uniform float raystep;		// minimum ray step to take (in voxels)
#include INTEGRATION


#ifdef DEPTH
uniform bool fxcam;		// is the fxcamDepth valid?
uniform sampler2DRect fxcamDepth;	// previous z-buffer
#endif

void main() {
#ifdef DEBUGPLANE
    // vertex shader sets each type of plane to a different color
    FragColor = DebugColor;
    return;
#endif

    // perspective-safe ray direction
    vec3 p = Vo.xyz/Vo.w;
    vec3 d = normalize(p*Eo.w - Eo.xyz);

    // find ray exit point from next slicing plane (or far plane)
    vec4 backPlane = vec4(0.,0.,-1.,1.)*gl_ModelViewProjectionMatrix;
#ifdef SLICES
    // fraction from back to front corner (0 to 1)
    float z = dot(normalize(backPlane.xyz),p)/length(volumeSize) + .5;

    // convert into slice number
    int slice = int(float(SLICES)*z-.5+1e-4);

    // move the back plane up if we're in front of the backmost slice
    if (slice > 0) {
	float offset = (float(slice)-.5)/float(SLICES)-.5;
	backPlane.w = -offset*length(backPlane.xyz)*length(volumeSize);
    }
#endif
    float exit = -(dot(backPlane.xyz,p)+backPlane.w)/dot(backPlane.xyz,d);

#ifdef BOX
    // intersect ray with box edges
    vec3 box = ((step(0.,d)-.5)*volumeSize - p)/d;
    box *= step(0.,box);	// avoid NaN for axis-aligned parallel view
    exit = min(exit,min(box.x,min(box.y,box.z)));
#endif

#ifdef CLIP
    // also intersect with user clipping planes
    for(int i=0; i<gl_MaxClipPlanes; ++i) {
	vec4 plane = gl_ClipPlane[i]*gl_ModelViewMatrix;
	float p_d = dot(plane.xyz,d);
	if (p_d<0.)
	    exit = min(exit, -(dot(plane.xyz,p)+plane.w)/p_d);
    }
#endif

#ifdef DEPTH
    // finally, check against previous z-buffer
    if (fxcam) {
	float zbuf = texture2DRect(fxcamDepth,gl_FragCoord.xy).r;
	vec4 plane = vec4(0.,0.,gl_DepthRange.diff,
			  (gl_DepthRange.near+gl_DepthRange.far-2.*zbuf)) 
	    * gl_ModelViewProjectionMatrix;
	exit = min(exit, -(dot(plane.xyz,p)+plane.w)/dot(plane.xyz,d));
    }
#endif

#ifdef DEBUGEXIT
    // show exit distance
    FragColor = vec4(vec3(exit/length(volumeSize)),1.);
    return;
#endif

    // start value on ray
    vec3 pPos = vec3(0); vec4 pVal=vec4(0), pCol=vec4(0);
    vec4 color = integration(p, 0., pPos, pVal, pCol);

#ifdef RANDSTEP
    // shrink factor to take a random partial first step
    // based on Hammersley sequence for interleaved bits of x&y
    uvec2 v = uvec2(gl_FragCoord.xy);
    uint ham = 0u; float hamMax = 1;
    ham = ham<<2u | v.x<<1u&2u | v.y&1u; v>>=1u; hamMax *= 4.;
    ham = ham<<2u | v.x<<1u&2u | v.y&1u; v>>=1u; hamMax *= 4.;
    ham = ham<<2u | v.x<<1u&2u | v.y&1u; v>>=1u; hamMax *= 4.;
    float start = float(ham+1u)/hamMax;
#else
    float start=1.;
#endif

    // adjust to size of ray step
    float scaledStep = raystep*length(volumeSize)/length(voxelSize);

    // loop through volume
    vec4 result = vec4(0.);
    float step, s; int i;
    for(i=0, step=scaledStep*start, s=step, p+=step*d;
	i<MAXSTEPS && s<exit; 
	++i, s+=step, p+=step*d) {

	// look up in volume, pass through transfer function, and accumulate
	color = integration(p, opticalScale*step, pPos, pVal, pCol);
	result = result + (1.-result.a)*color;
	step=scaledStep;

	// exit if opaque -- by setting s to exit, won't add final step
 	if (result.a >= 1.) i=MAXSTEPS;
    }

    // last partial step
    if (i != MAXSTEPS) {
	float laststep = exit-(s-step);	 // how big is the last step?
	p += (laststep - step) * d;  // undo last increment and do correct one

	// look up in volume, pass through transfer function, and accumulate
	color = integration(p, opticalScale*laststep, pPos, pVal, pCol);
	result = result + (1.-result.a)*color;
    }

    FragColor = result;
}
