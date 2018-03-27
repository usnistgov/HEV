
//////////////////
// Code for GkBoundary transfer function gk1.
//

// Required Externals:
//    volScalar          func:   float    volScalar

uniform sampler1D gk1_GkB_RGB;
uniform sampler1D gk1_GkB_ALPHA;
uniform float gk1_GkB_Range[4];

vec4 gk1 ()
{
    float val = volScalar ();
    float p = (val+gk1_GkB_Range[2])*gk1_GkB_Range[3];
    color = texture1D (gk1_GkB, p);
    color.a = (color.a+alphaOffset()) * alphaScale();
    return color;
} // end of gk1

//
// end of code for GkBoundary transfer function gk1.
//////////////////
