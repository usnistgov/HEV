varying vec3 Te;		// line tangent in eye space
varying vec3 Ve;		// view vector in eye space
varying vec3 Le;		// light position in eye space

void main() {
    // transform position and tangent
    gl_Position = ftransform();
    vec4 Pe = gl_ModelViewMatrix * gl_Vertex;
    Te = gl_NormalMatrix * gl_Normal;

    // compute light & view vectors
    vec4 Ee = gl_ProjectionMatrixInverse*vec4(0.,0.,-1.,0.);
    Ve = normalize(Ee.xyz*Pe.w - Pe.xyz*Ee.w);
    Le = normalize(gl_LightSource[0].position.xyz*Pe.w -
		   Pe.xyz*gl_LightSource[0].position.w);

    // pass vertex color through
    gl_FrontColor = gl_Color;
}
