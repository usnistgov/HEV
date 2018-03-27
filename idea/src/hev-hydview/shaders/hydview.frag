#version 330

in vec4 v_position;
in vec3 v_normal;

out vec4 frag_color;

uniform vec3 cameraPosition;
uniform vec4 mcolor;
uniform float refindex;
uniform float shininess;

const float PI = 3.14159265359f;

void main() {
  vec3 L = normalize(vec3(0, -sqrt(2.), sqrt(2.))); // light pos
  vec3 V = normalize(cameraPosition - v_position.xyz);
  vec3 H = normalize(V + L);

  vec3 N = normalize(v_normal);

  float N_L = max(0., dot(N, L));
  float N_V = max(0., dot(N, V));
  float N_H = max(0., dot(N, H));
  float V_H = max(0., dot(V, H));

  float refl0 = (refindex - 1.) / (refindex + 1.);
  refl0 *= refl0;

  float var = shininess;
  float D = pow(max(0., N_H), 2./var - 2.) / var;
  float G = sqrt(var / PI);
  G = 1. / ((N_L * (1 - G) + G) * (N_V * (1 - G) + G));
  float F = refl0 + (1 - refl0) * pow(1. - V_H, 5.);

  vec3 color = mix(vec3(mcolor.rgb) * (1 - F), vec3(D * G), F) * N_L;
  frag_color = vec4(color, mcolor.a);
}

