//
// Volume rendering ray caster fragment program. 
//
// This code must be run through the C preprocessor.  Various preprocessor
// symbols (macros) control what code is part of the final fragment program.
//
// The following preprocessor symbols should be provided:
//
//    INTEGRATION_FUNC_FN   file containing integration code
//    MAXSTEPS      maximum number of steps along ray; default is 1000
//    SLICES        number of slices for slice rendering; default is 0
//
//    Each of these preprocessor symbols enables functionality based on whether 
//    it is defined:
//
//    DEPTH       depth texture for embedded polygonal objects
//    BOX         ray intersection with vol box (why wouldn't you do this?)
//    CLIP        clipping plane handling
//    RANDSTEP    random initial step along ray
//    DEBUGPLANE  sets each type of plane to a different color
//    DEBUGEXIT   early exit; currently, color is exit distance
//    LIGHT_OPAQUE apply lighting to opaque samples
//
//    The transfer function must be provided in a separate source file. It
//    should be of the form  vec4 transFuncName ().  It gets the data that
//    it needs by calling functions that must be provided. These functions
//    are referred to as the transfer function support functions.  Here are
//    the preprocessor symbols related to the transfer function:
//
//    TRANSFER_FUNC_FN          Name of the transfer function source file
//    TRANSFER_FUNC_NAME        Name of the transfer function 
//    USE_STD_TF_SUPPORT_FUNCS  Indicates whether or not to use the
//                              standard set of transfer function support
//                              functions; default is 1 (true)
//    ALT_TF_SUPPORT_FUNCS_FN   This is used only if USE_STD_TF_SUPPORT_FUNCS
//                              indicates that the standard support functions
//                              are NOT to be used.  In that case, this symbol
//                              gives the name of a source file containing
//                              alternative transfer function support functions.
//
//    The transfer function support functions are a set of small functions
//    that enable the transfer function to access (or derive) the data that
//    it needs. The names of these functions have also be parameterized 
//    with preprocessor symbols. These all have defaults, so they do not
//    need to be specified unless there is a need to change them.
//
//              Preproc Name      Default     
//    vec3      VOL_POS           volPos     return curr xyz position along ray
//    vec3      VOL_PREV_POS      volPrevPos return previous xyz pos along ray
//    float     VOL_SCALAR        volScalar  return volume scalar data value at
//                                           the current position along ray
//    vec3      VOL_GRAD          volGrad    return gradiant vector of volume
//                                           scalar at current postion
//                                         
//

// The QUOTE macro will put quotes around the value of a preprocessor symbol
#define QUOTE_INNER(name) #name
#define QUOTE(macro) QUOTE_INNER(macro)


#ifndef SLICES
#define SLICES 0
#endif



#ifndef USE_STD_TF_SUPPORT_FUNCS
#define USE_STD_TF_SUPPORT_FUNCS 1
#endif

#ifndef VOL_POS
#define VOL_POS volPos
#endif

#ifndef VOL_PREV_POS
#define VOL_PREV_POS volPrevPos
#endif

#ifndef VOL_SCALAR
#define VOL_SCALAR volScalar
#endif

#ifndef VOL_GRAD
#define VOL_GRAD volGrad
#endif

#ifndef VOL_GRAD_CALC
#define VOL_GRAD_CALC volGradCalc
#endif

#ifndef VOL_GRAD_MAG
#define VOL_GRAD_MAG volGradMag
#endif

#ifndef VOL_GRAD_MAG_CALC
#define VOL_GRAD_MAG_CALC volGradMagCalc
#endif

#ifndef ALPHA_OFFSET
#define ALPHA_OFFSET alphaOffset
#endif

#ifndef ALPHA_SCALE
#define ALPHA_SCALE alphaScale
#endif




//
// MAXSTEPS gives the maximum number of steps to take along any ray
#ifndef MAXSTEPS
#define MAXSTEPS 1000
#endif


//////////////////////////////////////
// Some uniforms

// This uniform tells us when each user-supplied clip plane is enabled.
uniform bool iris_ClipPlaneEnabled[6];                                 

// volume is centered from -VolumeSize/2 .. VolumeSize/2
uniform vec3 VolumeSize;

// dimensions in voxels in each direction
uniform vec3 VoxelSize;

// does the scalar volume vary continuously?
uniform bool VolumeIsContinuous;

// opticalScale scales between geometric size and optical depth
//   increasing opticalScale makes the object more opaque
//   decreasing opticalScale makes the object more transparent
uniform float OpticalScale;

uniform float RayStep;              // minimum ray step to take (in voxels)

uniform float AlphaScale;
uniform float AlphaOffset;

#ifdef DEPTH
uniform bool fxcam;                 // is the fxcamDepth valid?
uniform sampler2DRect fxcamDepth;   // previous z-buffer
#endif


// This contains the 3D volume scalar data.
uniform sampler3D VolTxtrSc0; 
uniform sampler3D VolTxtrSc0Grad; 
uniform sampler3D VolTxtrSc0GradMag; 

//////////////////////////////////////
// in / out variables for fragment shader


in vec4 Eo;             // eye location in object space
in vec4 Vo;             // position in object space
in vec4 DebugColor;     // color each plane for debugging

out vec4 FragColor;

/////////////////////////////////////

// Some global variables

// The current xyz position of the ray traversal within the volume
vec3 CurrPosition;

// The previous xyz position of the ray traversal within the volume
vec3 PrevPosition;

vec3 RayDir;

float Ambient = 0.15;


// Is the initial sample on a clipping plane?
bool OnClippingPlane = false;
vec3 ClippingPlaneNormal;


//////////////////////////////
// Define some standard functions that may be used by the transfer function

#if USE_STD_TF_SUPPORT_FUNCS

float ALPHA_SCALE ()
{
    return AlphaScale;
} // end of ALPHA_SCALE ()


float ALPHA_OFFSET ()
{
    return AlphaOffset;
} // end of ALPHA_OFFSET()



// Note that CurrPosition and PrevPosition must be updated elsewhere, probably
// in the ray-traversal code

// Return the current volume position in ray traversal
vec3 VOL_POS ()
{
    return CurrPosition;
}  // end of VOL_POS

// Return the previous volume position in ray traversal
vec3 VOL_PREV_POS ()
{ 
    return PrevPosition;
}  // end of VOL_PREV_POS

// Return the scalar data value for the volume at the current position
float VOL_SCALAR ()
{
    return texture3D (VolTxtrSc0, 0.5+CurrPosition/VolumeSize).x;
}  // end of VOL_SCALAR


float VOL_SCALAR (vec3 pos)
{
    return texture3D (VolTxtrSc0, 0.5+pos/VolumeSize).x;
}  // end of VOL_SCALAR

float VOL_GRAD (vec3 pos)
{
    return texture3D (VolTxtrSc0Grad, 0.5+pos/VolumeSize).x;
}  // end of VOL_GRAD


// Return the gradiant vector of the volume scalar field at the current position
vec3 VOL_GRAD_CALC_OLD ()
{
    // Compute gradient: could preprocess to store in
    // volume (say val.xyz=normal, val.w=actual scalar value) for
    // now, do an incredibly inefficient extra six volume accesses.
    vec4 d = vec4 (0.5*VolumeSize/VoxelSize, 0.);
    
    return vec3 (
            VOL_SCALAR(CurrPosition-d.xww) - VOL_SCALAR(CurrPosition+d.xww),
            VOL_SCALAR(CurrPosition-d.wyw) - VOL_SCALAR(CurrPosition+d.wyw),
            VOL_SCALAR(CurrPosition-d.wwz) - VOL_SCALAR(CurrPosition+d.wwz)  );
}  // end of VOL_GRAD_CALC





vec3 VOL_GRAD_CALC ()
    {
    // If we're at the edge of the volume, we'll return the normal of the face.

    vec3 vp = VOL_POS ();
    vec3 dv = abs (vp/VolumeSize - 0.5);

    // If it's at the edge of the volume, then use the face normal
    if (dv.x < 0.00001)
        {
        return vec3 (1, 0, 0);
        }
    else if (dv.y < 0.00001)
        {
        return vec3 (0, 1, 0);
        }
    else if (dv.z < 0.00001)
        {
        return vec3 (0, 0, 1);
        }


    dv = abs (vp/VolumeSize + 0.5);
    if (dv.x < 0.00001)
        {
        return vec3 (-1, 0, 0);
        }
    else if (dv.y < 0.00001)
        {
        return vec3 (0, -1, 0);
        }
    else if (dv.z < 0.00001)
        {
        return vec3 (0, 0, -1);
        }

    // Compute gradient: Could preprocess to store in
    // volume (say val.xyz=normal, val.w=actual scalar value) for
    // now, do an incredibly inefficient extra six volume accesses.
    // This is OK for performance because it's only done once per opaque frag.
    vec4 d = vec4 (0.5*VolumeSize/VoxelSize, 0.);
    vec3 grad = 
      vec3 (
            VOL_SCALAR(CurrPosition-d.xww) - VOL_SCALAR(CurrPosition+d.xww),
            VOL_SCALAR(CurrPosition-d.wyw) - VOL_SCALAR(CurrPosition+d.wyw),
            VOL_SCALAR(CurrPosition-d.wwz) - VOL_SCALAR(CurrPosition+d.wwz)  );

    return grad;
    }  // end of VOL_GRAD_CALC









float VOL_GRAD_MAG (vec3 pos)
{
    return texture3D (VolTxtrSc0GradMag, 0.5+pos/VolumeSize).x;
}  // end of VOL_GRAD_MAG


float VOL_GRAD_MAG ()
{
    return VOL_GRAD_MAG (CurrPosition);
}  // end of VOL_GRAD_MAG


// Return the gradiant vector of the volume scalar field at the current position
float VOL_GRAD_MAG_CALC ()
{
    return length ( VOL_GRAD_CALC () );
}  // end of VOL_GRAD_MAG_CALC







#else

// Use alternative transfer function support functions
#include QUOTE(ALT_TF_SUPPORT_FUNCS_FN)

#endif

//////////////////////////////
// transfer function should be of the form  vec4 TRANFER_FUNC_NAME ().  
// It gets the data that it needs from the transfer function support functions.
//

#include QUOTE(TRANSFER_FUNC_FN)

// end of included transfer function code
//////////////////////////////


// Adjust alpha based on offset and scale
float alphaAdjust (float a)
    {
    return (a+alphaOffset()) * alphaScale();
    }  // end of alphaAdjust

// Determine if the indicated position is opaque based on the transfer func.
bool posIsOpaque (vec3 pos)
    {

    if (
        (abs(pos.x) > (VolumeSize.x/2)) ||
        (abs(pos.y) > (VolumeSize.y/2)) ||
        (abs(pos.z) > (VolumeSize.z/2))   )
        {
        return false;
    }

    // The following call to TRANSFER_FUNC_NAME
    // evaluates the TF at CurrPosition,
    // so we need to set CurrPosition to pos, the reset it when
    // we're done.

    vec3 saveCurrPos = CurrPosition;

    CurrPosition = pos;
    float a = alphaAdjust(TRANSFER_FUNC_NAME().a);
    CurrPosition = saveCurrPos;

    return (a >= 1);
    }  // end of posIsOpaque

// Derive a normal for an opaque voxel. 
// This function assumes:
//    - scalar field is discontinuous (nearest neighbor filtering)
//    - the position "inside" is opaque and "outside" is not
//    - rayDir is the direction of the cast ray
vec3 opaqueVoxelNormal (vec3 inside, vec3 outside, vec3 rayDir)
    {

    // If we already know that we're entering on a clipping plane
    // then we already know the normal.
    if (OnClippingPlane)
        {
        return ClippingPlaneNormal;
        }

    // The idea is that we look for the position between 
    // "inside" and "outside" that is on the face of the voxel,
    // then we return the axis-aligned normal to that voxel face
    // that points back along the ray direction.

    vec3 inPos = inside;
    vec3 outPos = outside;

    // Do a binary search to get close to the point at which the
    // segment changes from transparent to opaque.
    //
    // JGH: Maybe we should look for a change in color as well?
    //      Or maybe we should simply look for the intersection of
    //      the ray with the current voxel boundary.

    for (int i=0; i < 16; i++)
        {
        vec3 midPos = mix (inPos, outPos, 0.5);
        if (posIsOpaque (midPos))
            {
            inPos =  midPos;
            }
        else
            {
            outPos = midPos;
            }
        }

    // we transform the position from world coordinates to
    // fractional voxel index coordinates
    vec3 fracIndex = (inPos/VolumeSize+0.5) * VoxelSize;
    fracIndex = fracIndex - floor (fracIndex);

    // fracIndex is the distance to nearest face in each direction
    fracIndex = min (fracIndex, 1-fracIndex);

    // now figure out the normal
    vec3 norm;

    // find which face the point is closest to
    // make normal for that face pointing contrary to ray direction
    if (fracIndex.x <= fracIndex.y)
        {
        if (fracIndex.x <= fracIndex.z)
            {
            if (rayDir.x > 0)
                {
                norm = vec3 (-1, 0, 0);
                }
            else
                {
                norm = vec3 (1, 0, 0);
                }
            }
        else
            {
            if (rayDir.z > 0)
                {
                norm = vec3 (0, 0, -1);
                }
            else
                {
                norm = vec3 (0, 0, 1);
                }
            }
        }
    else
        {
        if (fracIndex.y <= fracIndex.z)
            {
            if (rayDir.y > 0)
                {
                norm = vec3 (0, -1, 0);
                }
            else
                {
                norm = vec3 (0, 1, 0);
                }
            }
        else
            {
            if (rayDir.z > 0)
                {
                norm = vec3 (0, 0, -1);
                }
            else
                {
                norm = vec3 (0, 0, 1);
                }
            }
        }

    // that's it!

    return norm;
    } // end of opaqueVoxelNormal


vec3 quickGuessNorm ()
    {
    // Make a guess about the normal of a surface at the current position 
    // in the volume.

    vec3 vp = VOL_POS ();
    vec3 dv = abs (vp/VolumeSize - 0.5);

    // If it's at the edge of the volume, then use the face normal
    if (dv.x < 0.00001)
        {
        return vec3 (1, 0, 0);
        }
    else if (dv.y < 0.00001)
        {
        return vec3 (0, 1, 0);
        }
    else if (dv.z < 0.00001)
        {
        return vec3 (0, 0, 1);
        }


    dv = abs (vp/VolumeSize + 0.5);
    if (dv.x < 0.00001)
        {
        return vec3 (-1, 0, 0);
        }
    else if (dv.y < 0.00001)
        {
        return vec3 (0, -1, 0);
        }
    else if (dv.z < 0.00001)
        {
        return vec3 (0, 0, -1);
        }

    // If it's not at a face, just use the position itself.
    // This is clearly bogus, but it's a guess.
    return vp;
    }  // end of quickGuessNorm


//////////////////////////////////////////
//
// Lighting of opaque samples.


// Apply lighting to opaque samples based on a calculated normal
vec4 applyOpaqueLighting (vec4 color, vec3 xyz, vec3 prevXyz)
    {

    if (color.a < 1.0)
        {
        return color;
        }



    // This code implements a simple lighting model for opaque samples.
    // It modifies a vec4 variable named color.

    // First we gather the data we need from outside of this function:
    vec3 p = xyz; // current position in volume
    vec4 pPrev = vec4 (prevXyz, 1); // previous position in volume


    vec3 normal;
    // Generate a normal for the scalar field
    if (VolumeIsContinuous)
        {
        // if the scalar field varies continuously then the
        //   normal = gradiant of vol continuous scalar field at the curr pos
        normal = VOL_GRAD_CALC ();
        }
    else
        {
        // Gradiant is not defined if volume is not continuous,
        // so we calculate the normal to the edge of the voxel.
        // The idea is that p is inside an opaque voxel, but
        // pPrev is not in an opaque voxel.
        normal = opaqueVoxelNormal (p, pPrev.xyz, normalize (xyz - prevXyz));
        }

    if ( all(equal(normal, vec3(0)) ) )
        {
        // if we have a zero gradient, then we need to fix it.
        normal = volPos ();
        }

    vec3 N = normalize (normal);

    // vector from current point in volume to light & viewer
    vec4 Lp = gl_ModelViewMatrixInverse*gl_LightSource[0].position;
    vec4 Vp = -gl_ModelViewProjectionMatrixInverse[2];
    vec3 L = normalize(Lp.xyz-p*Lp.w);
    vec3 V = normalize(Vp.xyz-p*Vp.w);
    vec3 H = normalize(L+V);

    // cap at clipping planes
    vec4 near = vec4(0.,0.,1.,1.)*gl_ModelViewProjectionMatrix;
    if (dot(near,vec4(xyz,1)) <= 0.00002)
        {
        N = normalize(-near.xyz);
        }


    // compute diffuse & specular terms
    float diffuse = max(0.,dot(N,L));

    // In the following code, sref could be a uniform

    // Here is Marc's current version:
    // compute diffuse & normalized specular terms
    // for specular normalization, see e.g.
    // http://renderwonk.com/publications/s2010-shading-course/
    vec4 sref = vec4(.04,.04,.04, 32);
    float specular = pow(max(0,dot(N,H)), sref.a)*(sref.a+2)*.5;
    // modify color based on diffuse and specular and uniform Ambient
    color.rgb = 
        (1-Ambient) * diffuse * (color.rgb + sref.rgb*specular) + 
        Ambient     * color.rgb;

    return color;
    }  // end of applyOpaqueLighting


//////////////////////////////
//
// Derive color/alpha for the sample at CurrPosition based on the
// transfer function, alphaAdjustments, and optional lighting.
//
vec4 getSampleRGBA ()
{
    vec4 color = TRANSFER_FUNC_NAME ();

    // alpha adjustments
    color.a = alphaAdjust(color.a);

#ifdef LIGHT_OPAQUE
    // lighting:
    color = applyOpaqueLighting (color, volPos(), volPrevPos());
#endif

    return color;
} // getSampleRGBA


//////////////////////////////
// Integration code should define two functions:
//     void initRayIntegration() - called before beginning a ray traversal
//     vec4 integration (vec3 pos, float step) - returns rgba of a step
//              pos = position of this sample
//              step = size of the integration step
//     integration() should call the transfer function if necessary
//

#include QUOTE(INTEGRATION_FUNC_FN)

//////////////////////////////////////////

// The main function of the fragment program



void main() {

#ifdef DEBUGPLANE
    // vertex shader sets each type of plane to a different color
    FragColor = DebugColor;
    return;
#endif

    // perspective-safe ray direction
    CurrPosition = Vo.xyz/Vo.w;

    vec3 rayDir = normalize(CurrPosition*Eo.w - Eo.xyz);
    RayDir = rayDir;

    // put initial previous position half way between eye and surface
    PrevPosition = (CurrPosition + Eo.xyz/Eo.w)/2;


    // find ray exit point from next slicing plane (or far plane)
    vec4 backPlane = vec4(0.,0.,-1.,1.)*gl_ModelViewProjectionMatrix;

#if SLICES
    // fraction from back to front corner (0 to 1)
    float z = dot(normalize(backPlane.xyz),CurrPosition)/length(VolumeSize) + .5;

    // convert into slice number
    int slice = int(float(SLICES)*z-.5+1e-4);

    // move the back plane up if we're in front of the backmost slice
    if (slice > 0) {
        float offset = (float(slice)-.5)/float(SLICES)-.5;
        backPlane.w = -offset*length(backPlane.xyz)*length(VolumeSize);
    }
#endif

    float exit = 
      -(dot(backPlane.xyz,CurrPosition)+backPlane.w)/dot(backPlane.xyz,rayDir);

#ifdef BOX
    // intersect ray with box edges
    vec3 box = ((step(0.,rayDir)-.5)*VolumeSize - CurrPosition)/rayDir;
    box *= step(0.,box);        // avoid NaN for axis-aligned parallel view
    exit = min(exit,min(box.x,min(box.y,box.z)));
#endif

    OnClippingPlane = false;  // Is the starting point on a clipping plane?

#ifdef CLIP
    // also intersect with user clipping planes

    // adjust exit point
    for(int i=0; i<6; ++i) {
        if (iris_ClipPlaneEnabled[i])
            {
            vec4 plane = gl_ClipPlane[i]*gl_ModelViewMatrix;
            float p_d = dot(plane.xyz,rayDir);
            if (p_d<0.) exit = 
                         min(exit, -(dot(plane.xyz,CurrPosition)+plane.w)/p_d);
            }

    }

    // figure out whether entry point is on a user clipping plane
    for(int i=0; i<6; ++i) {
        if (iris_ClipPlaneEnabled[i])
            {
            vec4 plane = gl_ClipPlane[i]*gl_ModelViewMatrix;
            float p_d = dot(plane,Vo);
            if (abs(p_d) < 0.00002 )
                {
                OnClippingPlane = true;
                ClippingPlaneNormal = -normalize (plane.xyz);
                // FragColor = vec4 (1,0,0,1);
                // return;
                }
            }
    }  // end of loop over clipping planes

#endif


#ifdef DEPTH
    // finally, check against previous z-buffer
    if (fxcam) {
        float zbuf = texture2DRect(fxcamDepth,gl_FragCoord.xy).r;
        vec4 plane = vec4(0.,0.,gl_DepthRange.diff,
                (gl_DepthRange.near+gl_DepthRange.far-2.*zbuf)) 
                * gl_ModelViewProjectionMatrix;
        exit = min(exit, -(dot(plane.xyz,CurrPosition)+plane.w)/dot(plane.xyz,rayDir));
    }
#endif

#ifdef DEBUGEXIT
    // show exit distance
    FragColor = vec4(vec3(exit/length(VolumeSize)),1.);
    return;
#endif

    initRayIntegration ();

    // is the following call necessary?
    vec4 color = integration(CurrPosition, 0.);


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
    float scaledStep = RayStep*length(VolumeSize)/length(VoxelSize);

    // Find the first intersection of the ray with the volume
    FragColor = integration (CurrPosition, OpticalScale*scaledStep*start);

    if (FragColor.a < 1)
        {

        OnClippingPlane = false; 

        // loop over samples along rayDir 

        vec4 result = vec4(0.);
        float step, s; 
        int i;
        for(i=0, step=scaledStep*start, s=step, CurrPosition+=step*rayDir;
            i<MAXSTEPS && s<exit && result.a<1.;    //JGH: remove result.a < 1.?
            ++i, s+=step, CurrPosition+=step*rayDir) {

            // look up in volume, pass through transfer function, and accumulate
            color = integration(CurrPosition, OpticalScale*step);
            PrevPosition = CurrPosition;
            result = result + (1.-result.a)*color;
            step=scaledStep;

            // exit if opaque -- by setting s to exit, won't add final step
            if (result.a >= 1.) i=MAXSTEPS;
        }  // end of loop over samples on ray

        // last partial step
        if (i < MAXSTEPS) {
            // how big is the last step?
            float laststep = exit-(s-step);         

            // undo last increment and do correct one
            CurrPosition += (laststep - step) * rayDir;  

            // look up in volume, pass through transfer function, and accumulate
            color = integration(CurrPosition, OpticalScale*laststep);
            PrevPosition = CurrPosition;
            result = result + (1.-result.a)*color;
        }

    FragColor = result;


    }  // end of if (FragColor.a < 1)


}   // end of main

///////////////////////////////////////////////////

