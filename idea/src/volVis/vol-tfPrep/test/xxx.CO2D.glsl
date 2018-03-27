
//////////////////
// Code for RgbaLut1D transfer function CO2D.

uniform sampler1D CO2D_RgbaLut1D;
uniform float CO2D_RgbaLut1D_Range[2];

vec4 CO2D ()
{
    // Required Externals:
    //    volGrad            func:   vec3     volGrad
    //    volPos             func:   vec3     volPos
    //    volPosPrev         func:   vec3     volPosPrev
    //    volScalar          func:   float    volScalar

    vec4 color;
    float val = volScalar ();
    color = texture1D (
      CO2D_RgbaLut1D,
      (val - CO2D_RgbaLut1D_Range[0]) / 
      (CO2D_RgbaLut1D_Range[1] - CO2D_RgbaLut1D_Range[0]) );

// lighting:
#define VOL_POS volPos
#define VOL_POS_PREV volPosPrev
#define VOL_GRAD volGrad
#include "RgbaLut1D.lightOpaque.glsl"
// end of lighting

    return color;
} // CO2D

// end of code for RgbaLut1D transfer function CO2D.
//////////////////
