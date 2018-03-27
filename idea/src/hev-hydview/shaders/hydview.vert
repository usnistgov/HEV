#version 330

layout (location=0) in vec3 position;
layout (location=1) in vec3 normal;

out vec4 v_position;
out vec3 v_normal;

uniform mat4 projectionFromCamera;
uniform mat4 cameraFromWorld;
uniform mat4 worldFromModel;
uniform mat3 normalMatrix;
uniform vec4 clipPlane;

void main() {
  v_position = worldFromModel * vec4(position, 1.f);
  v_normal = normalize(normalMatrix * normal);
  gl_Position = projectionFromCamera * cameraFromWorld * v_position;

  //gl_ClipDistance[0] = dot(clipPlane, v_position);
}

