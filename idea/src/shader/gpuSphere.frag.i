//GLSL#version 150 compatibility
//
// Fragment shader for simple lit spheres

// April 25, 2013 - Steve modified this and gpuSphere.geom.i to pass sphere
//                  radius via the alpha. To enable, set fatpointSize Uniform
//                  to -1. The alpha will be hard coded to 1.0
//                  Usefull for a .savg file of points to be rendered as
//                  varying size spheres.

uniform vec4 iris_Viewport;	// corner and size of window

#include <sphere.glsl>		// sphere support functions
#include <sphere_frag.glsl> // sphere fragment shader support functions
#include <light.glsl>		// lighting support functions

uniform float fatpointSize = -1; // sphere radius (-1 = from alpha)
uniform float shininess=60;	// specular exponent (overrides material)

in vec4 center;			// sphere center
in vec2 zrange;			// min and max depth
in float aradius;               // used when radius passed via alpha.
                                // seems necessary because gl_Color.a 
                                // limited to 1.0 in frag shader

void main()
{
    // compute sphere
    vec4 Fe; vec3 Ne;
    float face;

    gl_FragDepth = sphereDraw(/* in */ gl_FragCoord, center, aradius, zrange, 
			      /* out */ Fe, Ne, face);

    // not strictly necessary, but saves computation
    if (gl_FragDepth > gl_DepthRange.far) discard;

    // sphere material color, override base color & specular
    gl_MaterialParameters mtl = gl_FrontMaterial;
    mtl.ambient = mtl.diffuse = gl_Color;
    mtl.specular = vec4(1);
    mtl.shininess = shininess;

    // OpenGL-style specular, preserving input alpha
    gl_FragColor = light(mtl, Ne, Fe);
    if (fatpointSize < 0) {
      gl_FragColor.a = 1;
    } else {
      gl_FragColor.a = gl_Color.a;
    }

}
