// track pointer position using data from uniformUpdater

#include <quat.glsl>

uniform vec4 hev_wandPos;
uniform vec4 hev_wandQuat;

void main() {
    vec4 V = gl_Vertex;

    // rotate, translate & scale
    V.xyz = QRotate(hev_wandQuat,V.xyz)*hev_wandPos.w + hev_wandPos.xyz*V.w;

    // transform to screen space
    gl_Position = gl_ModelViewProjectionMatrix * V;

    // give it some color
    vec4 Lo = gl_ModelViewMatrixInverse*gl_LightSource[0].position;
    vec3 L = normalize(Lo.xyz*V.w - V.xyz*Lo.w);
    vec3 N = QRotate(hev_wandQuat, gl_Normal);
    gl_FrontColor = gl_Color * (dot(N,L)*.5+.5);
}
