// vertex shader for NPR rocks in qvd-NPR visualization app
//GLSL#version 120
//GLSL#extension GL_EXT_gpu_shader4 : enable

#include "constant.glsl"
#include "uniform.glsl"
#include "varying.glsl"

#include <quat.glsl>

// center (xyz) and overall stress (w) of rock
// stored in 4x4 blocks, frame*ROCKS + rock
// assumes ROCKS previously defined with the number of rocks
uniform sampler2D rockData;
vec4 RockData(int frame, int rock)
{
    int index = frame*ROCKS+rock;
    int V = index / (ROCKDATASIZE*4);
    int U = (index - V*ROCKDATASIZE*4)>>4;
    int v = (index & 0xf)>>2;
    int u = index & 0x3;
    return texture2D(rockData, vec2(U*4+u,V*4+v)/float(ROCKDATASIZE));
}

// rock-coordinate location (xyz) and stress (w) for closest points
// stored in 4x4 blocks
// assumes ROCKS and POINTS previously defined
uniform sampler2D pointData;
vec4 PointData(int frame, int rock, int point)
{
    int index = point + POINTS*(rock + ROCKS*frame);
    int V = index / (POINTDATASIZE*4);
    int U = (index - V*POINTDATASIZE*4)>>4;
    int v = (index & 0xf)>>2;
    int u = index & 0x3;
    return texture2D(pointData, vec2(U*4+u,V*4+v)/float(POINTDATASIZE));
}

// assumes ROCKS previously defined with the number of rocks
// orientation quaternion of rock
// stored in 4x4 blocks, frame*ROCKS + rock
uniform sampler2D orientation;
vec4 Orientation(int frame, int rock)
{
    int index = frame*ROCKS+rock;
    int V = index / (ORIENTATIONSIZE*4);
    int U = (index - V*ORIENTATIONSIZE*4)>>4;
    int v = (index & 0xf)>>2;
    int u = index & 0x3;
    return texture2D(orientation, vec2(U*4+u,V*4+v)/float(ORIENTATIONSIZE));
}

//// main body of shader
void main()
{
    // place to accumulate final color
    vec4 col = vec4(1.);

    //// rock index
    int rock = int(gl_Vertex.w) - 1;

    //// position and view transforms
    // assumes previous definition of RockData(), frame and rock
    vec4 quat = Orientation(int(frame),rock);
    gl_TexCoord[0] = gl_TextureMatrix[0]*gl_MultiTexCoord0;

    // rotation
    Pr = gl_Vertex.xyz;
    Po = QRotate(quat,Pr);

    // center
    vec4 rockdata = RockData(frame,rock);
    vec4 Eo = gl_ModelViewProjectionMatrixInverse*vec4(0,0,-1.,0);
    Po += rockdata.xyz;
    stress = rockdata.a;

    // silhouette detection
    vec3 Vo = normalize(Eo.xyz-Po.xyz*Eo.w);
    vec3 No = QRotate(quat,gl_Normal);
    silhouette = dot(Vo,No);

    // location of number
    vec4 Ce = gl_ModelViewMatrix*(vec4(rockdata.xyz,1.));
    vec4 Pe = gl_ClipVertex = gl_ModelViewMatrix*vec4(Po,1.);
    Pn = 20.*(Pe.xy*Ce.w-Ce.xy*Pe.w)/(Pe.w*Ce.w);

    // final position
    gl_Position = gl_ModelViewProjectionMatrix*vec4(Po,1.);

    //// compute lighting
    vec3 Ne = normalize(gl_NormalMatrix*No);
    vec3 Le = normalize(gl_LightSource[0].position.xyz*Pe.w -
	gl_LightSource[0].position.w*Pe.xyz);
    col *= gl_LightSource[0].ambient + 
	gl_LightSource[0].diffuse*max(dot(Ne,Le),0.);

    ////////////////////////////////
    // stress at key frame
    float keystress = keyframe<0. ? stress : RockData(int(keyframe),rock).a;
    int key = keyframe<0. ? frame : int(keyframe); 

    // turn off silhouettes for solid rocks
    if (max(keystress,0.) >= stressCutoff)
	silhouette = 0.;

    // color
    if (showcolor > .5) {
	vec4 markCol = abs(frame-key)<10 ? keyCol : stressCol;
	col *= mix(baseCol, markCol, min(stress/stressScale,1.));
    }

    // compute hot spot locations
    if (showspots > .5) {
	for(int i=0; i<SPOTS; ++i) {
	    stressData[i] = PointData(frame,rock,i);
	}
    }

    // send color to fragment shader
    gl_FrontColor = col;
}
