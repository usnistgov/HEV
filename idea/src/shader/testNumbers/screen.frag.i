//GLSL#version 150 compatibility
// sample and test for printing screen-aligned numbers on a model

#include <number.glsl>

uniform vec4 iris_Viewport;	// corner and size of window

uniform float textSize = 8;	// scale of text

void main()
{
    // center position for numbers in screen space
    // transform object space origin to screen space:
    //   gl_ModelViewProjection * vec4(0,0,0,1)
    vec4 Cp = gl_ModelViewProjectionMatrix[3];
    
    // transform fragment coordinate back to projection space
    vec2 Fp = 2*(gl_FragCoord.xy - iris_Viewport.xy)/iris_Viewport.zw - 1;

    // printing coordinates, centered on Cp and scaled for text size
    vec2 pcoord = (Fp - Cp.xy/Cp.w)*textSize;

    // some values to print that change a bunch and look interesting
    // should be something uniform or passed in from vertex or geometry shader
    vec4 pval = gl_ModelViewMatrixInverse[3];

    // print to 3 decimal places
    // line spacing is 1./3 units
    if (number(pval.x, 3, pcoord + vec2(.25,-3/6.)) > 0.5 ||
	number(pval.y, 3, pcoord + vec2(.25,-1/6.)) > 0.5 ||
	number(pval.z, 3, pcoord + vec2(.25, 1/6.)) > 0.5 ||
	number(pval.w, 3, pcoord + vec2(.25, 3/6.)) > 0.5) {

	gl_FragColor = vec4(1,0,0,1); // print in red
	return;
    }

    // color everything else normally
    gl_FragColor = gl_Color;
}
