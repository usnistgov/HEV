// default fragment shader for "things in motion"
//GLSL#version 150 compatibility

#include <light.glsl>

in vec4 Pe;
in vec3 Ne;

void main() {
    gl_MaterialParameters mtl = gl_FrontMaterial;
    mtl.diffuse = gl_Color;
    gl_FragColor = light(mtl, Ne, Pe);
}
