// integration step as a scaled constant color
// also used with preintegration transfer function, since that
// transfer function manages the value interpolation
// (but not step size scaling)

// return color and opacity after one ray step given:
//    pos = position of this sample
//    step = size of the integration step
vec4 integration(vec3 p, float step)
{
    // compute transfer function for this step
    vec4 xfer = getSampleRGBA ();
    xfer.a = clamp(xfer.a,0,1);

    // integrate along the step
    float scale = 1 - pow(1-xfer.a,step);
    xfer.rgb *= scale;
    xfer.a = scale;

    return xfer;
}  // end of integration ()

void initRayIntegration ()
{
}


