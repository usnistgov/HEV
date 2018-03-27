// dummy vertex shader for use with sphere.geom

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
    gl_FrontColor = gl_Color;
}
