#version 330

layout (location = 0) in vec2 vertex_pos;
layout (location = 1) in vec2 vertex_offset;

uniform vec2 screen;
uniform vec2 x_axis;
uniform vec2 y_axis;
uniform mat4 transform;
uniform mat4 projection;

void
main()
{
  vec4 pos = vec4(vertex_pos, 0.0, 1.0);
  gl_Position = projection * (transform * pos + vec4(vertex_offset, 0.0, 0.0));
}
