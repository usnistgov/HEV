//GLSL#version 150 compatibility

uniform vec4 iris_Viewport;	// corner and size of window
uniform float osg_FrameTime;	// for selection
uniform bool iris_ClipPlaneEnabled[6];

#include <sphere.glsl>		// sphere support functions
#include <sphere_frag.glsl> // sphere fragment shader support functions
#include <light.glsl>		// lighting support functions

uniform float fatpointSize;	// sphere radius
in vec4 center;			// sphere center
in vec2 zrange;			// min and max depth

// is the sphere hollow or solid?
#define SOLID 1

void main()
{
    // find both sphere interesections
    vec4 Fo0,Fo1; vec3 No0,No1; float d0, d1;
    sphereTest(gl_FragCoord, center, fatpointSize, zrange,
	       Fo0,No0,d0, Fo1,No1,d1);

    // transform to eye space for clip test
    vec4 Fe0 = gl_ModelViewMatrix * Fo0;
    vec4 Fe1 = gl_ModelViewMatrix * Fo1;

    // different handling for hollow or solid
    for(int i=0; i<gl_MaxClipPlanes; ++i) {
	if (iris_ClipPlaneEnabled[i]) {
#if SOLID
	    sphereClipSolid(gl_ClipPlane[i], Fe0,No0,d0, Fe1,No1,d1);
#else
	    sphereClipHollow(gl_ClipPlane[i], Fe0,No0,d0, Fe1,No1,d1);
#endif
	}
    }

#if SOLID
    // if solid, need to compute near intersection too
    // made more complex by the fact that the original front intersection
    // isn't necessarily even valid then.
    if (d0<0) {
	d0 = 0;
	No0 = -(gl_ModelViewProjectionMatrix[2]).xyz;
	Fe0 = gl_ProjectionMatrixInverse*
	    vec4((gl_FragCoord.xy-iris_Viewport.xy)/iris_Viewport.zw,-1,1);
    }
#endif
    

    // find closest unclipped intersection
    vec4 Fe; vec3 No; float face;
    gl_FragDepth = 1./0.;
    if (d0>=0) {
	Fe=Fe0; No=No0; gl_FragDepth=d0; face= 1;
    }
    else if (d1>=0) {
	Fe=Fe1; No=No1; gl_FragDepth=d1; face=-1;
    }

    // not strictly necessary, but saves computation
    if (gl_FragDepth > gl_DepthRange.far) discard;

    vec3 Ne = normalize(gl_NormalMatrix * No);

    // sphere material color
    gl_MaterialParameters mtl = gl_FrontMaterial;

    // per-sphere color
    mtl.ambient = mtl.diffuse = gl_Color;
    mtl.specular = vec4(1); mtl.shininess = 512;

    // OpenGL-style specular with fake environment map
    gl_FragColor = light(gl_LightSource[0], mtl, face*Ne, Fe);
    gl_FragColor = fakeEnv(gl_FragColor, face*Ne, Fe);
}
