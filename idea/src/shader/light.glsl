#ifndef light_glsl
#define light_glsl

// handy lighting functions

// note that in their current version, these
// do not include the spot light or distance attenuation terms.
//
// Lighting with all vectors passed to function
//   float diffuse(vec3 N, vec3 L);
//   float specular(vec3 N, vec3 L, vec3 V, float shininess);
// N, L and V should all be normalized and in the same space
//
// Lighting for a specific OpenGL light
//   vec4 diffuse(int num, vec3 Ne, vec4 Pe);
//   vec4 specular(int num, vec3 Ne, vec4 Pe, float shininess) 
//   vec4 light(int num, vec3 Ne, vec4 Pe, gl_MaterialParameters mat);
// Ne and Pe are the normal and surface position in eye space. mat and
// shininess are material parameters (which can be the OpenGL state,
// but don't have to be)
//
// Combined lighting for all OpenGL lights
//    vec4 light(vec3 Ne, vec4 Pe, gl_MaterialParameters mat);

uniform bool iris_LightEnabled[8];

// diffuse given unit normal and unit light vector
float diffuse(vec3 N, vec3 L) {
    return max(dot(N,L),0.);
}

// specular given unit normal, light vector and view vector, along
// with surface shininess (= specular exponent)
float specular(vec3 N, vec3 L, vec3 V, float shininess) {
    vec3 H = normalize(L+V);
    float NH = dot(N,H);
    if (NH <= 0.)
        return 0.;
    return pow(NH,shininess);
}

// diffuse for OpenGL light and material
// given unit normal in eye space and point on surface in eye space
vec4 diffuse(gl_LightSourceParameters lgt, gl_MaterialParameters mtl,
	     vec3 N, vec4 P) {
    vec3 L = normalize(lgt.position.xyz * P.w - lgt.position.w * P.xyz);
    return lgt.diffuse * mtl.diffuse * diffuse(N,L);
}

// specular for OpenGL light and material
// given unit normal and point on surface in eye space
vec4 specular(gl_LightSourceParameters lgt, gl_MaterialParameters mtl,
	      vec3 Ne, vec4 Pe) 
{
    vec3 Le = normalize(lgt.position.xyz * Pe.w - lgt.position.w * Pe.xyz);
    vec3 Ve = normalize(gl_ProjectionMatrixInverse[2].w * Pe.xyz -
			gl_ProjectionMatrixInverse[2].xyz * Pe.w);
    return lgt.specular * mtl.specular * specular(Ne, Le, Ve, mtl.shininess);
}

// total light contribution for OpenGL light and material
// given unit normal and point on surface in eye space
vec4 light(gl_LightSourceParameters lgt, gl_MaterialParameters mtl,
	   vec3 Ne, vec4 Pe)
{
    vec3 Le = normalize(lgt.position.xyz * Pe.w - lgt.position.w * Pe.xyz);
    vec3 Ve = normalize(gl_ProjectionMatrixInverse[2].w * Pe.xyz -
			gl_ProjectionMatrixInverse[2].xyz * Pe.w);

    return lgt.ambient * mtl.ambient 
	+ lgt.diffuse * mtl.diffuse * diffuse(Ne, Le) 
	+ lgt.specular * mtl.specular * specular(Ne, Le, Ve, mtl.shininess);
}

// Total light contribution for all OpenGL lights. 
// Somewhat inefficient since it does not have access to flags to know 
// which lights are enabled
vec4 light(gl_MaterialParameters mtl, vec3 Ne, vec4 Pe) {
    vec4 color = mtl.emission + mtl.ambient * gl_LightModel.ambient;
    for(int i=0; i<gl_MaxLights; ++i)
    {
        if (iris_LightEnabled[i])
	  color += light(gl_LightSource[i], mtl, Ne, Pe);
    }
    return color;
}

// physically plausible lighting contribution for OpenGL light and material
// See Lazarov, "Physically-Based Lighting in Call of Duty: Black
// Ops", Advances in Real-Time Rendering in Games course, SIGGRAPH 2012
vec4 physLight(gl_LightSourceParameters lgt, gl_MaterialParameters mtl,
	       vec3 Ne, vec4 Pe)
{
    // lighting vectors
    vec3 Le = normalize(lgt.position.xyz * Pe.w - lgt.position.w * Pe.xyz);
    vec3 Ve = normalize(gl_ProjectionMatrixInverse[2].w * Pe.xyz -
			gl_ProjectionMatrixInverse[2].xyz * Pe.w);
    vec3 He = normalize(Le+Ve);

    // clamped dot products
    float n_l = max(0.,dot(Ne,Le));
    float n_v = max(0.,dot(Ne,Ve));
    float n_h = max(0.,dot(Ne,He));
    float v_h = dot(Ve,He);

    // Schlick/Smith shadowing approximation a = 1/sqrt(s*pi/4 + pi/2)
    float a=inversesqrt(0.785398163398*mtl.shininess + 1.5707963268);
    float shad = 1./((n_l*(1.-a) + a) * (n_v*(1.-a) + a));
    float spec = .5*(mtl.shininess + 2.) * pow(n_h,mtl.shininess);
    vec4 fres = mtl.specular + (1.-mtl.specular) * pow(1.-v_h, 5.);

    return lgt.ambient * mtl.ambient +
	(lgt.diffuse * mtl.diffuse + lgt.specular * spec*fres*shad) * n_l;
}

// Simple fake high dynamic range Fresnel environment
// ground brown and below 1, sky bluish and brighter than 1
vec4 fakeEnv(vec4 baseColor, vec3 Ne, vec4 Pe) 
{
    // just use the y component of the reflection vector
    vec3 Ve = normalize(gl_ProjectionMatrixInverse[2].w * Pe.xyz -
		       gl_ProjectionMatrixInverse[2].xyz * Pe.w);
    float n_v = dot(Ne,Ve);
    float horizon = -Ve.y + 2.*n_v*Ne.y;

    // reflection color is a blend of brown to bright blue
    vec3 reflection = mix(vec3(.2,.5,.1),vec3(2.,2.8,3.6), 
			  smoothstep(-.2,0.,horizon));

    // blend using Fresnel reflectance, keep alpha from base color
    float fresnel = mix(pow(1.-dot(Ve,Ne),5.), 1., 0.03);
    return vec4(mix(baseColor.rgb, reflection, fresnel), baseColor.a);
}

// blend back and forth between two colors to indicate selection
vec4 selectBlink(vec4 color0, vec4 color1, float time)
{
    return mix(color0, color1, abs(mod(.5*time,2.) - 1.));
}

#endif
