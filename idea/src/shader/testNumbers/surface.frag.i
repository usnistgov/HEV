//GLSL#version 150 compatibility
// sample and test for printing object-aligned numbers on a model

#include <number.glsl>

uniform float textSize = 16;	// scale of text

void main()
{
    // printing coordinates, centered on (0.5,0.5) and scaled for text size
    vec2 pcoord = (gl_TexCoord[0].xy - vec2(0.5,0.5))*textSize;

    // some values to print that change a bunch and look interesting
    // should be something uniform or passed in from vertex or geometry shader
    vec4 pval = gl_ModelViewMatrixInverse[3];

    // print to 3 decimal places
    // line spacing is 1./3 units
    if (number(pval.x, 3, pcoord + vec2(0,-3/6.)) > 0.5 ||
	number(pval.y, 3, pcoord + vec2(0,-1/6.)) > 0.5 ||
	number(pval.z, 3, pcoord + vec2(0, 1/6.)) > 0.5 ||
	number(pval.w, 3, pcoord + vec2(0, 3/6.)) > 0.5) {

	gl_FragColor = vec4(1,0,0,1); // print in red
	return;
    }

    // color everything else normally
    gl_FragColor = gl_Color;
}
