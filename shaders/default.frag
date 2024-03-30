#version 330

layout (location = 0) out vec4 frag_color;

uniform vec2 screen;

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

void
main()
{
  vec2 pos = gl_FragCoord.xy;

  pos /= screen;
  pos *= 2.0;
  pos += vec2(-1.0, -1.0);
  pos.y = -pos.y;

  float angle = atan(pos.y, pos.x);
  if (angle < 0)
    angle += 2 * M_PI;
  angle /= 2 * M_PI;

  frag_color = vec4(color_point(angle), 1.0);
}
