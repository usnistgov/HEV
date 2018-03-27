// vertex shader for "things in motion"
//GLSL#version 330 compatibility

#include "tim.glsl"

in float object;		 // object number
uniform unsigned int frame; // frame number

out vec4 Pe;
out vec3 Ne;

void main() {
    unsigned int obj = unsigned int(object+0.5);

    // position each object
    vec3 p = gl_Vertex.xyz/gl_Vertex.w;
    Ne = gl_Normal;
    float t = timXform(p, Ne, obj, frame);

    // regular transform
    Pe = vec4(p, 1);
    gl_Position = gl_ModelViewProjectionMatrix * Pe;
    gl_ClipVertex = gl_ModelViewMatrix * Pe;
    Ne = normalize(gl_NormalMatrix * Ne);

    // pass through vertex data
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_TexCoord[1] = gl_MultiTexCoord1;
    gl_TexCoord[2] = gl_MultiTexCoord2;
    gl_TexCoord[3] = gl_MultiTexCoord3;

    gl_FrontColor = gl_Color;
}

