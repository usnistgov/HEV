#version 120
varying vec3 gsVertex;
void main( void ) {
    vec3 outColor = gsVertex * 0.5 + 0.5; // scale & bias to 0-1
    gl_FragColor = vec4(outColor,1.0);
}

