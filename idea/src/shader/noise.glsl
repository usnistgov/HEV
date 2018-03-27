#ifndef noise_glsl
#define noise_glsl

//////////////////////////////////////////////////////////////////////
// A subset of the noise functions from my 2005 Graphics Hardware
// paper: "Modified Noise for Evaluation on Graphics Hardware".
// - Marc Olano
//
// Externally usable functions:
//   float noiseT(float p);
//   float noiseT(vec2 p);
//   float noiseT(vec3 p);
// 1D, 2D and 3D Perlin-like noise functions.
// Like the original Perlin function, these return results in the
// range -1 to 1, with an average value of 0, passing through 0 at
// each integer input value, and between 1/2 and 1 cycle per input
// unit. Scale the input parameter before calling to get larger or
// smaller noise effects. These functions cycle every NTmod (=61)
// units in the input space. 
//
// Required texture data:
//   uniform sampler2D noiseTexture;
// Should refer to the noise.png texture.


// noise texture
uniform sampler2D noiseTexture;

// need to regenerate noise.png if these constants change
const float NTmod = 61.;	// modulus in random hash
const vec4 NTcoef = vec4(13.,17.,3.,1.); // coefficients for hash

float noiseT(float p) {
    return texture2D(noiseTexture, vec2(p/NTmod,0)).g*2.-1.;
}
float noiseT(vec2 p) {
    return texture2D(noiseTexture, p/NTmod).g*2.-1.;
}
float noiseT(vec3 p) {
    // integer and fractional components of input, and slice blend function
    float i = floor(p.z), f = fract(p.z), sf = (3.-2.*f)*f*f;

    // hash z & z+1 for noise offsets
    vec2 h = mod(NTcoef.z*(i+vec2(0,1.)),NTmod);
    h = mod(h*h,NTmod);

    // lookup noise and blend slices
    vec2 g0 = texture2D(noiseTexture, (p.xy+vec2(0,h.x))/NTmod).gb*2.-1.;
    vec2 g1 = texture2D(noiseTexture, (p.xy+vec2(0,h.y))/NTmod).gb*2.-1.;
    return mix(g0.r+g0.g*f, g1.r+g1.g*(f-1.), sf);
}


#endif
