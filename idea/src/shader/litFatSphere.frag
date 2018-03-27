// point fragment shader to compute ray/sphere intersection

uniform vec4 iris_Viewport;	// corner and size of window
uniform float fatpointSize=0.01;// size of spheres in view space

uniform float shininess=60.;	// size of specular highlights

varying vec3 Po;		// true center of sphere

void main() {
    // avoid NVIDIA compiler bug with gl_ModelViewProjectionMatrixInverse
    mat4 MVP_i = gl_ModelViewMatrixInverse*gl_ProjectionMatrixInverse;

    // compute Fo = fragment location in object space
    // Eo = point on near plane between eye and fragment in object space
    // really just two matrix multiplies, but reuse common terms
    vec2 vs = iris_Viewport.zw, vc = iris_Viewport.xy + .5*vs;
    float fmn = gl_DepthRange.diff, fpn = gl_DepthRange.far+gl_DepthRange.near;

    vec4 Eo = MVP_i[0]*(gl_FragCoord.x-vc.x)*2./vs.x
        +     MVP_i[1]*(gl_FragCoord.y-vc.y)*2./vs.y - MVP_i[2] + MVP_i[3];
    vec4 Fo = Eo + MVP_i[2]*((gl_FragCoord.z-gl_DepthRange.near)*2./fmn);

    // Eo and Fo relative to sphere center
    // guaranteed to have w>0, so go ahead and divide to save computation
    vec3 Fc = Fo.xyz/Fo.w - Po, Ec = Eo.xyz/Eo.w - Po;

    // plug ((1-t)*Ec + t*Fc) into quadric & solve for t
    float r2 = fatpointSize*fatpointSize;
    float FF = dot(Fc,Fc)-r2, EF = dot(Ec,Fc)-r2, EE = dot(Ec,Ec)-r2;
    float d = EF*EF-EE*FF;	// algebraically simplified discriminant
    if (d<0.) discard;		// no real root == no intersection

    // solve quadratic to find first intersection in front of near plane
    float u = EE-2.*EF+FF, t = (EE-EF-sqrt(d))/u;
    float face=1.;		// 1 or -1 for which face
    if (t<0.) {		   // behind near plane: try next intersection
      t = (EE-EF+sqrt(d))/u;
      face = -1.;
    }
    if (t<0.) discard;		// both intersections behind near plane
    Fc = (1.-t)*Ec + t*Fc;

    // Lighting Calculations

    // still centered at sphere, so easy to compute normal for shading
    vec3 N = normalize(face*gl_NormalMatrix*Fc);
    vec4 Fe = gl_ModelViewMatrix*vec4(Fc+Po,1.);

    vec3 global_ambient = gl_Color.xyz*gl_LightModel.ambient;
    vec3 ambient = vec3(0,0,0);
    vec3 diffuse = vec3(0,0,0);
    vec3 specular = vec3(0,0,0);
    float attenuation = 1.0;
    float spotlight = 1.0;

    //for every light
    for(int i=0; i<8; i++){
      vec3 l = gl_LightSource[i].position.xyz*Fe.w 
             - Fe.xyz*gl_LightSource[i].position.w;
      float dist = length(l);
      vec3 L = normalize(l);
      float NdL = max(0.,dot(N,L));
      attenuation = 1.0;

      //if the fragment is facing the light
      if(NdL > 0.0){
        //if the light is a spotlight
        if(gl_LightSource[i].spotCosCutoff>=0.0){
          spotlight = dot(normalize(gl_LightSource[i].spotDirection), -L);
          if(spotlight > gl_LightSource[i].spotCosCutoff){
            spotlight = pow(spotlight, gl_LightSource[i].spotExponent);
          }else{
            spotlight = 0.0;
          }
        }else{
          spotlight = 1.0;
        }
        if(gl_LightSource[i].position.w != 0.0){
          attenuation = spotlight / (gl_LightSource[i].constantAttenuation +
                                     gl_LightSource[i].linearAttenuation * dist +
                                     gl_LightSource[i].quadraticAttenuation * dist * dist);
        }
        
        specular += attenuation*gl_Color.xyz*gl_LightSource[i].specular*pow(max(dot(N, normalize(-normalize(Fe)+L)),0.0),shininess);
      }
      diffuse += attenuation*gl_Color.xyz*gl_LightSource[i].diffuse*NdL;
      ambient += attenuation*gl_Color.xyz*gl_LightSource[i].ambient;
    }
    gl_FragColor = vec4(global_ambient+ambient+diffuse+specular, gl_Color.w);

    // project back to screen space to find fragment depth
    vec4 Fp = gl_ProjectionMatrix*Fe;
    gl_FragDepth = .5*fmn*Fp.z/Fp.w + .5*fpn;
}
