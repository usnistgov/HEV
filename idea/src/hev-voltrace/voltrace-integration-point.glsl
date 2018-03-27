// integration step as a scaled constant color
// also used with preintegration transfer function, since that
// transfer function manages the value interpolation
// (but not step size scaling)

// return color and opacity after one ray step given:
//    pos = position of this sample
//    step = size of the integration step
//    pVal = previous value
//    pPos, pCol not used
vec4 integration(vec3 p, float step,
		 vec3 pPos, inout vec4 pVal, vec4 pCol) 
{
    // compute transfer function for this step
    vec4 xfer = transfer(p, pVal);
    xfer.a = clamp(xfer.a,0,1);

    // integrate along the step
    float scale = 1 - pow(1-xfer.a,step);
    xfer.rgb *= scale;
    xfer.a = scale;

    return xfer;
}
