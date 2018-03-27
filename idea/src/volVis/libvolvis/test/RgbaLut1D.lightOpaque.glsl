

    // This code implements a simple lighting model for opaque samples.  
    // It modifies a vec4 variable named color.

    if (color.a >= 1.) 
    {
      // First we gather the data we need from outside of this function:
      vec3 p = VOL_POS ();                    // current position in volume
      vec4 pPrev = vec4 (VOL_PREV_POS (), 1); // previous position in volume
      // Normal = gradiant of vol scalar field at the curr pos
      vec3 N = normalize (VOL_GRAD ());       

      // vector from current point in volume to light & viewer
      vec4 Lp = gl_ModelViewMatrixInverse*gl_LightSource[0].position;
      vec4 Vp = -gl_ModelViewProjectionMatrixInverse[2];
      vec3 L = normalize(Lp.xyz-p*Lp.w);
      vec3 V = normalize(Vp.xyz-p*Vp.w);
      vec3 H = normalize(L+V);

      // cap at clipping planes
      vec4 near = vec4(0.,0.,1.,1.)*gl_ModelViewProjectionMatrix;
      if (dot(near,pPrev)<=0) N = normalize(-near.xyz);
      //
      for(int i=0; i<gl_MaxClipPlanes; ++i) 
      {
          vec4 plane = gl_ClipPlane[i]*gl_ModelViewMatrix;
          if (dot(plane, pPrev)<0) N = normalize(plane.xyz);
      }

      // compute diffuse & specular terms
      float diffuse = max(0.,dot(N,L));


      // In the following code, sref could be a uniform
#if 0
      // Here is the version JGH has been using:
      vec4 sref = vec4(1.,1.,1.,64.);
      float specular = pow(max(0.,dot(N,H)), sref.a);
      // modify color based on diffuse and specular
      color.rgb = diffuse*color.rgb + sref.rgb*specular;
#else
      // Here is Marc's current version:
      vec4 sref = vec4(.04,.04,.04, 32);
      float specular = pow(max(0,dot(N,H)), sref.a)*(sref.a+2)*.5;
      // modify color based on diffuse and specular
      color.rgb = diffuse*(color.rgb + sref.rgb*specular);
#endif

      }  // end of if (color.a >= 1.) : simple lighting model

