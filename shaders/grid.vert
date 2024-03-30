#version 330

layout (location = 0) in vec2 vpos;
layout (location = 1) in vec2 voffset;

uniform mat3x2 transform;

void
main()
{
  vec2 pos = (transform * vec3(vpos, 1.0)).xy + voffset;
  gl_Position = vec4(pos, 0.0, 1.0);
}
