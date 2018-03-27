#ifndef number_glsl
#define number_glsl

//////////////////////////////////////////////////////////////////////
// print numbers from within a shader
//
// Externally usable functions:
//   float number(float value, float digits, vec2 coord);
// Returns a per-pixel greyscale value for a printed number. This
// version only prints in a fixed decimal format similar to C's %f
// printf format.
// Arguments:
//    value = value to print
//    digits = number of digits to print to the right of the decimal
//    coord = pixel position in a 2D space for printing
// The decimal point will be placed at coord=0,0. The baseline will be
// at coord.y=0, and the number will be 1/3 unit high. Individual
// characters will be spaced at 4 characters per unit of coord.x. 
//
// Required texture data:
//    uniform sampler 2D numbers;
// Texture containing a grid of equally spaced numbers in the desired
// font. The basic texture layout is:
//   0123
//   4567
//   89.-

uniform sampler2D numbers;

// print val to d digits with decimal point at coord=vec2(0,0)
// line height = 1/3 unit, character width 1/4 unit
float number(float val, float d, vec2 coord) {
    // nothing to print outside the current line
    if (coord.y<0. || 3.*coord.y>1.) return 0.;

    // round value
    float aval = abs(val)+ .5*pow(.1,d);

    // find character index
    float c = floor(4.*coord.x);
    if (c > d+.5) return 0.;	// cut off after d decimal places
    coord.x -= .25*c;		// coord now spans a single character

    // what's my digit?
    float digit;
    if (c == 0.)
	digit=10.;		// 10th spot in texture = decimal
    else {
	if (c < 0.) c += 1.;	// 10^-c is now digit position
	digit = floor(mod(aval*pow(10.,c),10.));

	// find sign location
	float s = floor(c + log2(max(aval,1.))/log2(10.));
	if (s < 0.) {		// at sign or end of number
	    if (val<0. && s == -1.)
		digit = 11.;	// 11th spot in texture = minus sign
	    else
		return 0.;	// past end of number
	}
    }

    // adjust coordinate to this digit
    coord.y += .666-.333*floor(.25*digit);
    coord.x += .25*mod(digit,4.);

    return texture2D(numbers,coord).r;
}


#endif
