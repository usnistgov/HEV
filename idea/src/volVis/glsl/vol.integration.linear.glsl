// approximation for linear interpolation of color & alpha
// from Ken Moreland's PhD dissertation, UNM 2004
// "Fast High Accuracy Volume Rendering"

// return color and opacity after one ray step given:
//    pos = position of this sample
//    step = size of the integration step
//    pPos, pCol = previous position, value and color
vec3 IntegrationPrevPos;
vec4 IntegrationPrevCol;

vec4 integration(vec3 pos, float step)
{
    // compute transfer function for this step
    vec4 xfer = getSampleRGBA ();
    float s = step;

    // if we crossed alpha=1, do one secant method step to improve intersection
    // avoid for first integration, when step=0
    if (s>0 && IntegrationPrevCol.a<1 && xfer.a>1) {
        float frac = (1-IntegrationPrevCol.a)/(xfer.a-IntegrationPrevCol.a);
        vec3 p = mix(IntegrationPrevPos, pos,frac);
        xfer = getSampleRGBA ();
        s *= frac;
    }
    xfer.a = clamp(xfer.a,0.,1.);

    // approximation for integral along the step
    // simplified from Moreland's version by replacing his
    // 'psi' term = -alpha/log(1-alpha) with just alpha, which
    // is the limit value at both alpha=0 and alpha=1
    float avg = .5*(xfer.a+IntegrationPrevCol.a);
    float diff = xfer.a-IntegrationPrevCol.a;
    float alpha = 1 - pow(1.-avg-.108165*diff*diff, s);
    float beta = 1 - pow(1.-.27*xfer.a-.73*IntegrationPrevCol.a, s);
    vec3 color = xfer.rgb*(alpha-beta) + IntegrationPrevCol.rgb*beta;

    // save previous values for next step
    IntegrationPrevCol = xfer;
    IntegrationPrevPos = pos;

    return vec4(color, alpha);
}  // end of integration ()

void initRayIntegration ()
{
    IntegrationPrevPos = vec3 (0.);
    IntegrationPrevCol = vec4 (0.);
}  // end of initRayIntegration
