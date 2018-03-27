#version 120
varying vec3 vsVertex;
void main( void ) {
     vsVertex = gl_Vertex.xyz;
     gl_Position = gl_ModelViewProjectionMatrix*gl_Vertex;
  }
