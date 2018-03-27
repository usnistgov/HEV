
//////////////////
// Code for RgbaLut1D transfer function DefaultRgbaLut1D.

uniform sampler1D DefaultRgbaLut1D_RgbaLut1D;
uniform float DefaultRgbaLut1D_RgbaLut1D_Range[2];

vec4 DefaultRgbaLut1D ()
{
    // Required Externals:
    //    volGrad            func:   vec3     volGrad
    //    volPos             func:   vec3     volPos
    //    volPosPrev         func:   vec3     volPosPrev
    //    volScalar          func:   float    volScalar

    vec4 color;
    float val = volScalar ();
    color = texture1D (
      DefaultRgbaLut1D_RgbaLut1D,
      (val - DefaultRgbaLut1D_RgbaLut1D_Range[0]) / 
      (DefaultRgbaLut1D_RgbaLut1D_Range[1] - DefaultRgbaLut1D_RgbaLut1D_Range[0]) );

// lighting:
#define VOL_POS volPos
#define VOL_POS_PREV volPosPrev
#define VOL_GRAD volGrad
#include "RgbaLut1D.lightOpaque.glsl"
// end of lighting

    return color;
} // DefaultRgbaLut1D

// end of code for RgbaLut1D transfer function DefaultRgbaLut1D.
//////////////////
