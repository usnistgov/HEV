
//////////////////
// Code for GkBoundary transfer function gk2.
//

// Required Externals:
//    volScalar          func:   float    volScalar

uniform sampler1D gk2_GkB_RGB;
uniform sampler1D gk2_GkB_ALPHA;
uniform float gk2_GkB_Range[4];

vec4 gk2 ()
{
    float val = volScalar ();
    float p = (val+gk2_GkB_Range[2])*gk2_GkB_Range[3];
    color = texture1D (gk2_GkB, p);
    color.a = (color.a+alphaOffset()) * alphaScale();
    return color;
} // end of gk2

//
// end of code for GkBoundary transfer function gk2.
//////////////////
