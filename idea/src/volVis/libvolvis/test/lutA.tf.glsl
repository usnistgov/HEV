
//////////////////
// Code for RgbaLut1D transfer function DefaultRgbaLut1D.

uniform sampler1D DefaultRgbaLut1DRgbaLut1D;
uniform float DefaultRgbaLut1DRgbaLut1DRange[2];

vec4 DefaultRgbaLut1D ()
{
    # Required Externals:
    #    volGrad            func:   vec3     volGrad
    #    volPos             func:   vec3     volPos
    #    volPosPrev         func:   vec3     volPosPrev
    #    volScalar          func:   float    volScalar

    vec4 color;
    float val = volScalar ();
    color = texture1D (
      DefaultRgbaLut1DRgbaLut1D,
      (val-DefaultRgbaLut1DRgbaLut1DRange[0]) / (DefaultRgbaLut1DRgbaLut1DRange[1]-DefaultRgbaLut1DRgbaLut1DRange[0]) );

// lighting:
#define VOL_POS volPos
#define VOL_POS_PREV volPosPrev
#define VOL_GRAD volGrad
#include "volume.RgbaLut1D.lightOpaque.glsl"
// end of lighting

    return color;
} // DefaultRgbaLut1D

// end of code for RgbaLut1D transfer function DefaultRgbaLut1D.
//////////////////
