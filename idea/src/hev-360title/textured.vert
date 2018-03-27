#version 120

void main() {
    // transform position
    gl_Position = ftransform();
    gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;

    // copy through texture coordinates
    gl_TexCoord[0] = gl_TextureMatrix[0]*gl_MultiTexCoord0;
    gl_TexCoord[1] = gl_TextureMatrix[1]*gl_MultiTexCoord1;
    gl_TexCoord[2] = gl_TextureMatrix[2]*gl_MultiTexCoord2;
    gl_TexCoord[3] = gl_TextureMatrix[3]*gl_MultiTexCoord3;
    gl_TexCoord[4] = gl_TextureMatrix[4]*gl_MultiTexCoord4;
    gl_TexCoord[5] = gl_TextureMatrix[5]*gl_MultiTexCoord5;
    gl_TexCoord[6] = gl_TextureMatrix[6]*gl_MultiTexCoord6;
    gl_TexCoord[7] = gl_TextureMatrix[7]*gl_MultiTexCoord7;
}

