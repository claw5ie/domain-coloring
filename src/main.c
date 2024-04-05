#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>
#include <stddef.h>
#include <stdarg.h>
#include <math.h>
#include <assert.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float  f32;
typedef double f64;

#include "utils.c"

int g_screen_width = 800;
int g_screen_height = 600;

float g_left = (-8.0);
float g_right = 8.0;
float g_bottom = (-6.0);
float g_top = 6.0;

struct QuadVertexArray
{
  GLuint vertex_array;
  GLuint vertex_buffer;

  GLuint program;

  GLint screen_loc;
  GLint x_axis_loc;
  GLint y_axis_loc;
  GLint transform_loc;
  GLint projection_loc;
};

typedef struct GlfwContext GlfwContext;
struct GlfwContext
{
  struct QuadVertexArray *quad;

  bool is_lmb_pressed;

  float x_mouse_pos;
  float y_mouse_pos;
};

void
update_coordinate_system(GlfwContext *ctx, float left, float right, float bottom, float top)
{
  float transform[4][4] = {
    { fabsf(right - left), 0, 0, 0 },
    { 0, fabsf(top - bottom), 0, 0 },
    { 0, 0, 1, 0 },
    { left, bottom, 0, 1 },
  };
  float projection[4][4] = {
    { 2.0 / (right - left), 0, 0, 0 },
    { 0, 2.0 / (top - bottom), 0, 0 },
    { 0, 0, 1, 0 },
    { -(right + left) / (right - left), -(top + bottom) / (top - bottom), 0, 1 },
  };

  glUseProgram(ctx->quad->program);

  glUniform2f(ctx->quad->screen_loc, g_screen_width, g_screen_height);
  glUniform2f(ctx->quad->x_axis_loc, left, bottom);
  glUniform2f(ctx->quad->y_axis_loc, right, top);
  glUniformMatrix4fv(ctx->quad->transform_loc, 1, GL_FALSE, (float *)transform);
  glUniformMatrix4fv(ctx->quad->projection_loc, 1, GL_FALSE, (float *)projection);
}

void
to_local_coordinates(float *x_dst, float *y_dst, float x, float y)
{
  float xx = x * (g_right - g_left) / g_screen_width;
  xx += g_left;
  float yy = y * (g_bottom - g_top) / g_screen_height;
  yy += g_top;

  *x_dst = xx;
  *y_dst = yy;
}

void
get_cursor_pos(GLFWwindow *window, float *x_dst, float *y_dst)
{
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  to_local_coordinates(x_dst, y_dst, x, y);
}

void
framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  float scale = (g_right - g_left) / (g_top - g_bottom) * ((float)height / width);
  float offset = (g_top + g_bottom) / 2;

  g_bottom -= offset;
  g_top -= offset;
  g_bottom *= scale;
  g_top *= scale;
  g_bottom += offset;
  g_top += offset;

  g_screen_width = width;
  g_screen_height = height;

  glViewport(0, 0, width, height);

  GlfwContext *ctx = glfwGetWindowUserPointer(window);
  assert(ctx);
  update_coordinate_system(ctx, g_left, g_right, g_bottom, g_top);
}

void
mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
  (void)mods;

  GlfwContext *ctx = glfwGetWindowUserPointer(window);
  assert(ctx);

  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
      get_cursor_pos(window, &ctx->x_mouse_pos, &ctx->y_mouse_pos);
      ctx->is_lmb_pressed = true;
    }
  else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
      float x_offset = 0, y_offset = 0;
      get_cursor_pos(window, &x_offset, &y_offset);

      x_offset -= ctx->x_mouse_pos;
      y_offset -= ctx->y_mouse_pos;

      g_left -= x_offset;
      g_right -= x_offset;
      g_bottom -= y_offset;
      g_top -= y_offset;

      update_coordinate_system(ctx, g_left, g_right, g_bottom, g_top);

      ctx->is_lmb_pressed = false;
    }
}

void
cursor_pos_callback(GLFWwindow *window, double x, double y)
{
  GlfwContext *ctx = glfwGetWindowUserPointer(window);
  assert(ctx);

  if (ctx->is_lmb_pressed)
    {
      float x_offset = 0, y_offset = 0;
      to_local_coordinates(&x_offset, &y_offset, x, y);
      x_offset -= ctx->x_mouse_pos;
      y_offset -= ctx->y_mouse_pos;

      update_coordinate_system(ctx, g_left - x_offset, g_right - x_offset, g_bottom - y_offset, g_top - y_offset);
    }
}

void
scroll_callback(GLFWwindow *window, double x, double y)
{
  (void)x;

  y = y < 0 ? 1 / 0.85 : y > 0 ? 0.85 : 0;

  GlfwContext *ctx = glfwGetWindowUserPointer(window);
  assert(ctx);

  float x_offset = 0, y_offset = 0;
  get_cursor_pos(window, &x_offset, &y_offset);

  g_left -= x_offset;
  g_right -= x_offset;
  g_bottom -= y_offset;
  g_top -= y_offset;

  g_left *= y;
  g_right *= y;
  g_bottom *= y;
  g_top *= y;

  g_left += x_offset;
  g_right += x_offset;
  g_bottom += y_offset;
  g_top += y_offset;

  update_coordinate_system(ctx, g_left, g_right, g_bottom, g_top);
}

int
main(void)
{
  if (!glfwInit())
    exit(EXIT_FAILURE);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(g_screen_width, g_screen_height, "Hello, World!", NULL, NULL);
  if (!window)
    exit(EXIT_FAILURE);

  glfwMakeContextCurrent(window);

  if (glewInit() != GLEW_OK)
    exit(EXIT_FAILURE);

  puts((const char *)glGetString(GL_VENDOR));
  puts((const char *)glGetString(GL_RENDERER));

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetCursorPosCallback(window, cursor_pos_callback);
  glfwSetScrollCallback(window, scroll_callback);

  struct QuadVertexArray quad = { 0 };

  {
    {
      float data[] = { 0, 0,
                       1, 0,
                       0, 1,
                       1, 1 };

      glCreateBuffers(1, &quad.vertex_buffer);
      glBindBuffer(GL_ARRAY_BUFFER, quad.vertex_buffer);
      glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

      glCreateVertexArrays(1, &quad.vertex_array);
      glBindVertexArray(quad.vertex_array);

      glBindBuffer(GL_ARRAY_BUFFER, quad.vertex_buffer);
      glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
      glEnableVertexAttribArray(0);
    }

    {
      GLuint vertex_shader = create_shader(GL_VERTEX_SHADER, "./shaders/default.vert");
      GLuint fragment_shader = create_shader(GL_FRAGMENT_SHADER, "./shaders/default.frag");
      quad.program = create_program(vertex_shader, fragment_shader);

      glDeleteShader(vertex_shader);
      glDeleteShader(fragment_shader);
    }

    {
      quad.screen_loc = glGetUniformLocation(quad.program, "screen");
      quad.x_axis_loc = glGetUniformLocation(quad.program, "x_axis");
      quad.y_axis_loc = glGetUniformLocation(quad.program, "y_axis");
      quad.transform_loc = glGetUniformLocation(quad.program, "transform");
      quad.projection_loc = glGetUniformLocation(quad.program, "projection");

      assert(quad.screen_loc != -1);
      assert(quad.x_axis_loc != -1);
      assert(quad.y_axis_loc != -1);
      assert(quad.transform_loc != -1);
      assert(quad.projection_loc != -1);
    }
  }

  GlfwContext ctx = {
    .quad = &quad,
    .is_lmb_pressed = false,
    .x_mouse_pos = 0,
    .y_mouse_pos = 0,
  };

  glfwSetWindowUserPointer(window, &ctx);

  update_coordinate_system(&ctx, g_left, g_right, g_bottom, g_top);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(1.0, 1.0, 1.0, 1.0);

  for (GLenum error; (error = glGetError()) != GL_NO_ERROR; )
    {
      switch (error)
        {
        case GL_INVALID_ENUM:
          fputs("GL_INVALID_ENUM", stderr);
          break;
        case GL_INVALID_VALUE:
          fputs("GL_INVALID_VALUE", stderr);
          break;
        case GL_INVALID_OPERATION:
          fputs("GL_INVALID_OPERATION", stderr);
          break;
        case GL_STACK_OVERFLOW:
          fputs("GL_STACK_OVERFLOW", stderr);
          break;
        case GL_STACK_UNDERFLOW:
          fputs("GL_STACK_UNDERFLOW", stderr);
          break;
        case GL_OUT_OF_MEMORY:
          fputs("GL_OUT_OF_MEMORY", stderr);
          break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
          fputs("GL_INVALID_FRAMEBUFFER_OPERATION", stderr);
          break;
        case GL_CONTEXT_LOST:
          fputs("GL_CONTEXT_LOST", stderr);
          break;
        case GL_TABLE_TOO_LARGE:
          fputs("GL_TABLE_TOO_LARGE", stderr);
          break;
        }
      putchar('\n');
    }

  while (!glfwWindowShouldClose(window))
    {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

      glBindVertexArray(quad.vertex_array);
      glUseProgram(quad.program);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

      glfwSwapBuffers(window);
      glfwPollEvents();
    }

  glfwTerminate();
  exit(EXIT_SUCCESS);
}
