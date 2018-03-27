varying vec4 Ce;		// sphere center in eye space
varying vec4 Ee;		// eye location in eye space
varying float select;		// 1 if point is selected & should blink

uniform float osg_FrameTime;	// time since app start in seconds
uniform vec4 iris_Viewport;	// center and size of window
uniform float fatpointSize;	// size of spheres

void main() {
    float nf = gl_DepthRange.near + gl_DepthRange.far;

    // transform screen-space location to view space
    // inverts transform from section 2.11.1 of the OpenGL spec
    vec4 Ps = vec4((gl_FragCoord.xy-iris_Viewport.xy)*2./iris_Viewport.zw - 1.,
		   (2.*gl_FragCoord.z - nf) / gl_DepthRange.diff,
		   1.);
    vec4 Pe ;
    Pe = gl_ModelViewProjectionMatrixInverse * Ps;

    // distance from sphere center to point on surface
    float dist = distance(Pe.xyz/Pe.w, Ce.xyz/Ce.w) / fatpointSize;

    // trim points based on distance from center
    if (dist > 1.) discard;

    // depth based on distance from center solving x^2+y^2+z^2=1
    float depth = sqrt(1.-dist*dist);

    // modulate color as if lit by a light at the viewer
    gl_FragColor = vec4(gl_Color.rgb*depth,gl_Color.a);

    // set blink for selection
    float blend = select>.5 ? 1. : abs(mod(osg_FrameTime,4)-2);
    gl_FragColor = mix(vec4(0),gl_FragColor,blend);

    // translate along view vector to adjust pixel depth
    vec3 Ve = normalize(Ee.xyz*Pe.w - Pe.xyz*Ee.w);
    Pe.xyz += Pe.w * fatpointSize * depth * Ve;
    Ps = gl_ModelViewProjectionMatrix * Pe;

    gl_FragDepth = (gl_DepthRange.diff*Ps.z/Ps.w + nf)/2.;
}
