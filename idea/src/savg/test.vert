attribute vec3 color ;
varying vec4 fragmentColor ;
void main() 
{

    const vec3 LightPos = vec3( 3., 5., 10. ) ;

    vec3 TransNorm = normalize( gl_NormalMatrix * gl_Normal ) ;

    vec3 ECposition = vec3 (gl_ModelViewMatrix * gl_Vertex ) ;

    float LightIntensity = dot( normalize( LightPos - ECposition), TransNorm ) ;

    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex ;

    fragmentColor = vec4(color, 1.0) ;
    gl_FrontColor = gl_Color * LightIntensity ;

}

