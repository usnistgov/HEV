// volume data from a 3D texture, scaled to physical quantities
// domain -1..1 in each axis, range as returned by 3D texture

uniform sampler3D volumeTexture; // 3D volume
uniform vec4 volmin, volrange;	// define mapping from texture to data range

vec4 voldata(vec3 p) {
    return texture3D(volumeTexture, .5+p/volumeSize)*volrange + volmin;
}

