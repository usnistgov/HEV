// fragment shader for NPR rocks in qvd-NPR visualization app
//GLSL#version 120
//GLSL#extension GL_EXT_gpu_shader4 : enable

#include "constant.glsl"
#include "uniform.glsl"
#include "varying.glsl"

// 3D perlin noise
#include <noise.glsl>

// numeric output
#include <number.glsl>

//// main body of shader
void main()
{
    // place to accumulate final color
    vec4 col = gl_Color;

    // outline or solid rock?
    if (abs(silhouette) > fwidth(silhouette))
	col.a=0.;

    // compute hot spot locations and colors
    if (showspots > .5) {
	for(int i=0; i<SPOTS; ++i) {
	    // check stress level and distance
	    vec3 spot = stressData[i].xyz;
	    float stress = stressData[i].a;
	    float dist = min(1.,distance(Po,spot)/spotSize);

	    // color based on distance to spot
	    if (stress > spotCutoff) col = mix(spotCol,col,dist);
	}
    }

    // write stress value for current frame on rock
    if (silhouette == 0. && showtext > .5) {
	col.rgb = mix(col.rgb,textCol.rgb, number(stress,2.,Pn.xy));
    }

    // discard transparent pixels
    if (col.a < .5) discard;

    gl_FragColor = col;
}
