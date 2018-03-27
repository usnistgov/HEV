// volume data from a straight lookup into a 3D texture
// domain -1..1 in each axis, range as returned by 3D texture

uniform sampler3D volumeTexture; // 3D volume

vec4 voldata(vec3 p) {
    return texture3D(volumeTexture, .5+p/volumeSize);
}

