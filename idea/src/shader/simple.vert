// simple vertex shader approximating fixed-function lighting
// except for distance attenuation and spot falloff

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
    
    // do lighting for one light
    vec3 N = normalize(gl_NormalMatrix*gl_Normal);
    vec3 V = normalize(gl_ClipVertex.xyz*gl_ProjectionMatrixInverse[2].w -
		       gl_ProjectionMatrixInverse[2].xyz*gl_ClipVertex.w);

    gl_FrontColor = gl_FrontMaterial.emission +
	gl_FrontMaterial.ambient*gl_LightModel.ambient;
    gl_BackColor = gl_BackMaterial.emission +
	gl_BackMaterial.ambient*gl_LightModel.ambient;

    for(int i=0; i<gl_MaxLights; ++i) {
	vec3 L = normalize(gl_LightSource[i].position.xyz*gl_ClipVertex.w -
			   gl_ClipVertex.xyz*gl_LightSource[i].position.w);
	vec3 H = normalize(V+L);
	float diff = dot(N,L), spec=dot(N,H);

	gl_FrontColor += 
	    gl_FrontMaterial.ambient*gl_LightSource[i].ambient +
	    gl_FrontMaterial.diffuse*gl_LightSource[i].diffuse*max(0.,diff) +
	    gl_FrontMaterial.specular*gl_LightSource[i].specular
	    * (diff<0.?0.:pow(max(0.,spec),gl_FrontMaterial.shininess));
	gl_BackColor += 
	    gl_BackMaterial.ambient*gl_LightSource[i].ambient +
	    gl_BackMaterial.diffuse*gl_LightSource[i].diffuse*max(0.,-diff) +
	    gl_BackMaterial.specular*gl_LightSource[i].specular
	    * (diff>0.?0.:pow(max(0.,-spec),gl_BackMaterial.shininess));
    }
}
