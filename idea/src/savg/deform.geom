#version 120
#extension GL_EXT_geometry_shader4 : enable
uniform float osg_FrameTime;
varying in vec3 vsVertex[];
varying out vec3 gsVertex;
void main( void ) {
    
    // not a pass-through program
    for(float i = 0.0; i < 128.0; i += 1.0)
    {
        float u = i/127.0;
        vec4 pos = mix( gl_PositionIn[0], gl_PositionIn[1], u );
        pos.y += cos(u * 2.0 * 3.1415926 + osg_FrameTime) * 0.1;
        gsVertex = mix( vsVertex[0], vsVertex[1], u );
        gl_Position = pos;
        EmitVertex();
    } 
}

