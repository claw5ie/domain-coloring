#version 330

layout (location = 0) out vec4 frag_color;

uniform vec2 screen;
uniform vec2 x_axis;
uniform vec2 y_axis;

#define M_PI 3.1415926535897932384626433832795
#define M_TAU (2 * M_PI)

vec3
h_color_model(vec2 point)
{
  float angle = atan(point.y, point.x) * (3.0 / M_TAU);

  angle += 3.0 * float(angle < 0.0);

  vec3 color = vec3(0.0, 0.0, 0.0);

  if (angle < 1.0)
    color = vec3(1.0 - angle, angle, 0.0);
  else if (angle < 2.0)
    color = vec3(0.0, 2.0 - angle, angle - 1.0);
  else
    color = vec3(angle - 2.0, 0.0, 3.0 - angle);

  return color;
}

vec3
hl_color_model(vec2 point)
{
  float angle = atan(point.y, point.x) * (3.0 / M_TAU);
  float len = length(point);

  angle += 3.0 * float(angle < 0.0);

  vec3 color = vec3(0.0, 0.0, 0.0);

  if (angle < 1.0)
    color = vec3(1.0 - angle, angle, 0.0);
  else if (angle < 2.0)
    color = vec3(0.0, 2.0 - angle, angle - 1.0);
  else
    color = vec3(angle - 2.0, 0.0, 3.0 - angle);

  // float l = 4.0 / (1.0 + exp(-len)) - 2.0; // Clamps the interval to [0, 2[, but grows too quickly.
  // float l = atan(len) / (M_PI / 4.0); // Grows less than sigmoid, good avarage between sigmoid and hyperbole.
  float l = 2.0 - 2.0 / (len + 1.0);

  if (l > 1.0)
    color += (l - 1.0) * (vec3(1.0, 1.0, 1.0) - color);
  else
    color *= l;

  return color;
}

vec2
transform(vec2 p)
{
  return vec2(sin(p.x * p.x - p.y * p.y), 2 * p.x * p.y);
  // return p;
  // return p.x * vec2(p.x / p.y, p.y * p.x);
  // return vec2(p.x / p.y, p.y * p.x);
  // if (p.x + p.y < 0)
  //   return vec2(-sqrt(-(p.x + p.y)), 2 * p.x * p.y);
  // else
  //   return vec2(sqrt(p.x + p.y), 2 * p.x * p.y);
  // return vec2(p.x / p.y, p.y * p.x);
}

void
main()
{
  vec2 pos = gl_FragCoord.xy * (y_axis - x_axis) / screen + x_axis;

  frag_color = vec4(hl_color_model(transform(pos)), 1.0);
}
