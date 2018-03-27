#version 120
// point fragment shader to compute ray/sphere intersection

uniform float osg_FrameTime;	// time since app start in seconds
uniform vec4 iris_Viewport;	// corner and size of window
uniform float fatpointSize=1;	// size of spheres in view space
uniform float specular=64;	// specular power
uniform float shinyPoints=0;	// shiny or dull points?

varying vec3 Po;		// true center of sphere
varying float select;		// 1 if point is selected & should blink

void main() {
    // base color = surface color
    gl_FragColor = gl_Color;

    // avoid NVIDIA compiler bug with gl_ModelViewProjectionMatrixInverse
    mat4 MVP_i = gl_ModelViewMatrixInverse*gl_ProjectionMatrixInverse;

    // compute Fo = fragment location in object space
    // Eo = point on near plane between eye and fragment in object space
    // really just two matrix multiplies, but reuse common terms
    vec2 vs = iris_Viewport.zw, vc = iris_Viewport.xy + .5*vs;
    float fmn = gl_DepthRange.diff, fpn = gl_DepthRange.far+gl_DepthRange.near;

    vec4 Eo = MVP_i[0]*(gl_FragCoord.x-vc.x)*2./vs.x
	+     MVP_i[1]*(gl_FragCoord.y-vc.y)*2./vs.y - MVP_i[2] + MVP_i[3];
    vec4 Fo = Eo + MVP_i[2]*((gl_FragCoord.z-gl_DepthRange.near)*2./fmn);

    // Eo and Fo relative to sphere center
    // guaranteed to have w>0, so go ahead and divide to save computation
    vec3 Fc = Fo.xyz/Fo.w - Po, Ec = Eo.xyz/Eo.w - Po;

    // plug ((1-t)*Ec + t*Fc) into quadric & solve for t
    float r2 = fatpointSize*fatpointSize;
    float FF = dot(Fc,Fc)-r2, EF = dot(Ec,Fc)-r2, EE = dot(Ec,Ec)-r2;
    float d = EF*EF-EE*FF;	// algebraically simplified discriminant
    if (d<0.) discard;		// no real root == no intersection

    // solve quadratic to find first intersection in front of near plane
    float u = EE-2.*EF+FF, t = (EE-EF-sqrt(d))/u;
    float face=1.;		// 1 or -1 for which face
    if (t<0.) {		   // behind near plane: try next intersection
	t = (EE-EF+sqrt(d))/u;
	face = -1.;
    }
    if (t<0.) discard;		// both intersections behind near plane
    Fc = (1.-t)*Ec + t*Fc;

    // still centered at sphere, so easy to compute normal for shading
    vec3 Ne = normalize(face*gl_NormalMatrix*Fc);
    vec4 Ee = -gl_ProjectionMatrixInverse[2];
    vec4 Fe = gl_ModelViewMatrix*vec4(Fc+Po,1.);
    vec3 Le = normalize(gl_LightSource[0].position.xyz*Fe.w 
		       - Fe.xyz*gl_LightSource[0].position.w);
    vec3 Ve = normalize(Ee.xyz*Fe.w - Fe.xyz*Ee.w);
    vec3 He = normalize(Ve+Le);

    // diffuse & specular color
    float diff = max(0.,dot(Ne,Le));
    float spec = specular*pow(max(0.,dot(Ne,He)), specular);
    float fresnel = mix(pow(1+dot(-Ve,He),5),1,0.02);
    gl_FragColor.rgb = (gl_FragColor.rgb+fresnel*spec)*diff;

    // set blink for selection
    float blend = select<.5 ? 1. : abs(mod(osg_FrameTime,4)-2);
    gl_FragColor = mix(vec4(0),gl_FragColor,blend);

    if (shinyPoints>0.5) {
	// fake HDR horizon (sky brighter than ground)
	vec3 Re = reflect(-Ve,Ne);
	vec3 refl = mix(vec3(.2,.5,.1),vec3(2.,2.8,3.6),
			smoothstep(-.2,0.,Re.y));

	// blend using Schlick approximation to Fresnel reflectance
	float fresnel = mix(pow(1.+dot(-Ve,normalize(Re+Ve)),5.), 1., 0.02);
	gl_FragColor.rgb = mix(gl_FragColor.rgb,refl,fresnel);
    }

    // project back to screen space to find fragment depth
    vec4 Fp = gl_ProjectionMatrix*Fe;
    gl_FragDepth = .5*fmn*Fp.z/Fp.w + .5*fpn;
}
