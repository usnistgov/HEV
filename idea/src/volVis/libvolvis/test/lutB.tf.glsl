
//////////////////
// Code for RgbaLut1D transfer function noName.

uniform sampler1D noNameRgbaLut1D;
uniform float noNameRgbaLut1DRange[2];

vec4 noName ()
{
    # Required Externals:
    #    volScalar          func:   float    volScalar

    vec4 color;
    float val = volScalar ();
    color = texture1D (
      noNameRgbaLut1D,
      (val-noNameRgbaLut1DRange[0]) / (noNameRgbaLut1DRange[1]-noNameRgbaLut1DRange[0]) );
    return color;
} // noName

// end of code for RgbaLut1D transfer function noName.
//////////////////
