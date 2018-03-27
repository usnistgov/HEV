
//////////////////
// Code for RgbaLut1D transfer function myName.

uniform sampler1D myNameRgbLut1D;
uniform float myNameRgbLut1DRange[2];
uniform sampler1D myNameAlphaLut1D;
uniform float myNameAlphaLut1DRange[2];

vec4 myName ()
{
    # Required Externals:
    #    volScalar          func:   float    volScalar

    vec4 color;
    float val = volScalar ();
    val = log(val)/log(10.);
    color.rgb = texture1D ( 
      myNameRgbLut1D,
      (val-myNameRgbLut1DRange[0]) / (myNameRgbLut1DRange[1]-myNameRgbLut1DRange[0]) ).rgb;
    color.a = texture1D ( 
      myNameAlphaLut1D,
      (val-myNameAlphaLut1DRange[0]) / (myNameAlphaLut1DRange[1]-myNameAlphaLut1DRange[0]) ).x;
    return color;
} // myName

// end of code for RgbaLut1D transfer function myName.
//////////////////
