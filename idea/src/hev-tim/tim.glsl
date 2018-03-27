// variables and functions for "things in motion"

#ifndef tim_glsl
#define tim_glsl

#include <quat.glsl>

uniform unsigned int timFrames;	// total frames
uniform unsigned int timObjects; // total objects
uniform sampler2D timPosn;	// positions per object per frame
uniform sampler2D timQuat;	// orientation per object per frame

// return texture index given object and frame
// object and frame is turned into a linear index
// then decoded in Moreton/Z order
vec2 timIndex(unsigned int object, unsigned int frame) {
    // turn object and frame into a global integer index
    unsigned int index = object + timObjects*frame;

    // X is the even bits of index (where LSB=bit 0), Y is the odd bits 
    // take index: yxyxyxyx yxyxyxyx yxyxyxyx yxyxyxyx
    // extract and align x & y: 
    //    0x0x0x0x 0x0x0x0x 0x0x0x0x 0x0x0x0x
    //    0y0y0y0y 0y0y0y0y 0y0y0y0y 0y0y0y0y
    // pack bits of each:
    //    00000000 00000000 xxxxxxxx xxxxxxxx
    //    00000000 00000000 yyyyyyyy yyyyyyyy
    uvec2 coord = uvec2(index, index>>1u) & 0x55555555u;
    coord = (coord | coord>>1u) & 0x33333333u;
    coord = (coord | coord>>2u) & 0x0F0F0F0Fu;
    coord = (coord | coord>>4u) & 0x00FF00FFu;
    coord = (coord | coord>>8u) & 0x0000FFFFu;

    return (vec2(coord)+0.5)/vec2(textureSize(timPosn,0));
}

// look up orientation as quaternion
vec4 timQuaternion(vec2 index) {
    return texture2D(timQuat, index);
}

// look up orientation as matrix
mat3 timMatrix(vec2 index) {
    return QsMatrix(texture2D(timQuat, index));
}

// look up translation
vec4 timPosition(vec2 index) {
    return texture2D(timPosn, index);
}

// transform vertex and normal using quaternion
vec3 timVertex(vec4 quat, vec4 posn, vec3 vert) {
    return QsRotate(quat, vert) + posn.xyz;
}
vec3 timNormal(vec4 quat, vec3 norm) {
    return QsRotate(quat, norm);
}

// transform point and normal with matrix
vec3 timVertex(mat3 mat, vec4 posn, vec3 vert) {
    return mat*vert + posn.xyz;
}
vec3 timNormal(mat3 mat, vec3 norm) {
    return mat*norm;
}

// the whole shebang, vertex & normal in, transformed vertex & normal out
// returns 4th (user-specified) component of position texture
float timXform(inout vec3 vert, inout vec3 norm, 
	       unsigned int object, unsigned int frame)
{
    vec2 index = timIndex(object, frame);
    mat3 mat = timMatrix(index);
    vec4 posn = timPosition(index);
    vert = timVertex(mat, posn, vert);
    norm = timNormal(mat, norm);
    return posn.w;
}
#endif
