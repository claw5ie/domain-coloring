#version 330

layout (location = 0) out vec4 frag_color;

uniform vec2 screen;
uniform vec2 x_axis;
uniform vec2 y_axis;

#define M_PI 3.1415926535897932384626433832795

vec3
color_point(float angle)
{
  angle *= 3.0;

  if (angle < 1.0)
    {
      return vec3(1.0 - angle, angle, 0.0);
    }
  else if (angle < 2.0)
    {
      return vec3(0.0, 2.0 - angle, angle - 1.0);
    }
  else
    {
      return vec3(angle - 2.0, 0.0, 3.0 - angle);
    }
}

vec2
func(vec2 pos)
{
  return vec2(sin(pos.x * pos.x - pos.y * pos.y), 2 * pos.x * pos.y);
}

void
main()
{
  vec2 pos = gl_FragCoord.xy;

  pos /= screen;
  pos *= y_axis - x_axis;
  pos.y = -pos.y;
  pos += vec2(x_axis.x, y_axis.y);

  pos = func(pos);

  float angle = atan(pos.y, pos.x);
  if (angle < 0)
    angle += 2 * M_PI;
  angle /= 2 * M_PI;

  frag_color = vec4(color_point(angle), 1.0);
}
