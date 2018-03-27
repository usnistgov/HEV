varying vec3 Te;		// line tangent in eye space
varying vec3 Ve;		// view vector in eye space
varying vec3 Le;		// light position in eye space

void main() {
    // renormalize interpolated vectors
    vec3 Tn = normalize(Te);
    vec3 Vn = normalize(Ve);
    vec3 Ln = normalize(Le);
    vec3 Hn = normalize(Vn+Ln);

    // compute diffuse shading
    float diff = dot(Tn,Ln);
    diff = sqrt(1.-diff*diff);

    // compute specular shading
    float spec = dot(Tn,Hn);
//     spec = pow(sqrt(1.-spec*spec),gl_FrontMaterial.shininess);
    spec = pow(sqrt(1.-spec*spec),64.);

    // combine into final color
    gl_FragColor = gl_FrontMaterial.emission +
	gl_Color*(gl_LightSource[0].ambient + gl_LightSource[0].diffuse*diff) +
	vec4(1.,1.,1.,0.)*gl_LightSource[0].specular*spec;
}
