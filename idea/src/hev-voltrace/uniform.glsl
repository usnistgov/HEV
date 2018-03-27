// volume is centered from -volumeSize/2 .. volumeSize/2
uniform vec3 volumeSize;

// dimensions in voxels in each direction
uniform vec3 voxelSize;

// opticalScale scales between geometric size and optical depth
//   increasing opticalScale makes the object more opaque
//   decreasing opticalScale makes the object more transparent
uniform float opticalScale;

