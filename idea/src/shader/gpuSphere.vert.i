// pass-through vertex shader for gpuSphere

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
    gl_FrontColor = gl_Color;
}
