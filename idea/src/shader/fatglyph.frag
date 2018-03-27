#define PI 3.14159265
varying vec4 Ce;		// sphere center in eye space
varying vec4 Ee;		// eye location in eye space

uniform vec4 iris_Viewport;	// center and size of window
uniform float fatpointSize;	// size of spheres

varying float sides;		// number of sides (from gl_Normal.x)
varying float size;		// size of glyph (from gl_Normal.y)
varying float eyespace;		// 0 means use eye space, !0 means object space (from gl_Normal.z)

void main() {
    float nf = gl_DepthRange.near + gl_DepthRange.far;

    // transform screen-space location to view space
    // inverts transform from section 2.11.1 of the OpenGL spec
    vec4 Ps = vec4(
	(gl_FragCoord.xy-iris_Viewport.xy)*2./iris_Viewport.zw - 1., 
	(2.*gl_FragCoord.z - nf) / gl_DepthRange.diff, 1.);

    vec4 Pe = gl_ProjectionMatrixInverse * Ps;

    // distance from sphere center to point on surface
    float dist = distance(Pe.xyz/Pe.w, Ce.xyz/Ce.w) / size;

    float theta = atan(Pe.y*Ce.w-Ce.y*Pe.w,Pe.x*Ce.w-Ce.x*Pe.w);

    // trim points based on distance from center & set depth
    float depth = 0.0;

    // circle
    if (sides < 2.5) {
	if (dist > 1.) discard;
    	depth = sqrt(1.-dist*dist);	// solution to x^2+y^2+z^2=1
    }

    // general case- size is distance to vertex
    else {
        float d = dist*cos((mod(PI + sides*theta,2.0*PI)-PI)/sides)/cos(PI/sides);
	if (d > 1.) discard;
	depth = 1. - d;
    }

    // modulate color as if lit by a light at the viewer
    // let colors be more vibrant (eliminate black)
    gl_FragColor = vec4(gl_Color.rgb*(depth*.5+.5),gl_Color.a);

    // translate along view vector to adjust pixel depth
    vec3 Ve = normalize(Ee.xyz*Pe.w - Pe.xyz*Ee.w);
    Pe.xyz += Pe.w * size * depth * Ve;
    Ps = gl_ProjectionMatrix * Pe;

    gl_FragDepth = (gl_DepthRange.diff*Ps.z/Ps.w + nf)/2.;
}
