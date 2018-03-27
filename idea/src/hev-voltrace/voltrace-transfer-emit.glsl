// 1D transfer function for voltrace volume renderer
// For volume rendering with the emission/absorption model, in which
// each voxel emits light according to the transfer function and absorbs
// light passing through it according to the transfer alpha


// 1D transfer function mapping scalar values to color and opacity
uniform sampler2D volumeLUT;

// lookup-based emmission/absorption model transfer function
//    p = position of this sample
//    pVal = not used
vec4 transfer(vec3 p, vec4 pVal) {
    return texture2D(volumeLUT, voldata(p).rr);
}
