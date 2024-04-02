#version 330

layout (location = 0) in vec2 vertex_pos;

uniform vec2 screen;
uniform vec2 x_axis;
uniform vec2 y_axis;
uniform mat4 transform;
uniform mat4 projection;

void
main()
{
  gl_Position = projection * transform * vec4(vertex_pos, 0.0, 1.0);
}
