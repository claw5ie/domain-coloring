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

int SCREEN_WIDTH = 800;
int SCREEN_HEIGHT = 600;

float LEFT = (-8.0);
float RIGHT = 8.0;
float BOTTOM = (-6.0);
float TOP = 6.0;

typedef struct GlfwContext GlfwContext;
struct GlfwContext
{
  GLuint program;
  GLint screen_loc;
  GLint x_axis_loc;
  GLint y_axis_loc;
  GLint transform_loc;
  GLint projection_loc;
};

void
update_coordinate_system(GlfwContext *ctx)
{
  glUseProgram(ctx->program);

  glUniform2f(ctx->screen_loc, SCREEN_WIDTH, SCREEN_HEIGHT);
  glUniform2f(ctx->x_axis_loc, LEFT, BOTTOM);
  glUniform2f(ctx->y_axis_loc, RIGHT, TOP);

  {
    float matrix[4][4] = {
      { fabsf(RIGHT - LEFT), 0, 0, 0 },
      { 0, fabsf(TOP - BOTTOM), 0, 0 },
      { 0, 0, 1, 0 },
      { LEFT, BOTTOM, 0, 1 },
    };
    glUniformMatrix4fv(ctx->transform_loc, 1, GL_FALSE, (float *)matrix);
  }

  {
    float matrix[4][4] = {
      { 2.0 / (RIGHT - LEFT), 0, 0, 0 },
      { 0, 2.0 / (TOP - BOTTOM), 0, 0 },
      { 0, 0, 1, 0 },
      { -(RIGHT + LEFT) / (RIGHT - LEFT), -(TOP + BOTTOM) / (TOP - BOTTOM), 0, 1 },
    };
    glUniformMatrix4fv(ctx->projection_loc, 1, GL_FALSE, (float *)matrix);
  }
}

void
framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  float scale = (RIGHT - LEFT) / (TOP - BOTTOM) * ((float)height / width);
  float offset = (TOP + BOTTOM) / 2;

  BOTTOM -= offset;
  TOP -= offset;
  BOTTOM *= scale;
  TOP *= scale;
  BOTTOM += offset;
  TOP += offset;

  SCREEN_WIDTH = width;
  SCREEN_HEIGHT = height;

  glViewport(0, 0, width, height);

  GlfwContext *ctx = glfwGetWindowUserPointer(window);
  assert(ctx);
  update_coordinate_system(ctx);
}

int
main(void)
{
  if (!glfwInit())
    exit(EXIT_FAILURE);

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello, World!", NULL, NULL);
  if (!window)
    exit(EXIT_FAILURE);

  glfwMakeContextCurrent(window);

  if (glewInit() != GLEW_OK)
    exit(EXIT_FAILURE);

  puts((const char *)glGetString(GL_VENDOR));
  puts((const char *)glGetString(GL_RENDERER));

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  float quad[] = { 0, 0,
                   1, 0,
                   0, 1,
                   1, 1 };

  GLuint quad_array;
  glCreateVertexArrays(1, &quad_array);

  GLuint quad_buffer;
  glCreateBuffers(1, &quad_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, quad_buffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

  glBindVertexArray(quad_array);

  glBindBuffer(GL_ARRAY_BUFFER, quad_buffer);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
  glEnableVertexAttribArray(0);

  GLuint program = 0;

  {
    GLuint vertex_shader = create_shader(GL_VERTEX_SHADER, "./shaders/default.vert");
    GLuint fragment_shader = create_shader(GL_FRAGMENT_SHADER, "./shaders/default.frag");
    program = create_program(vertex_shader, fragment_shader);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
  }

  GLint screen_loc = glGetUniformLocation(program, "screen");
  GLint x_axis_loc = glGetUniformLocation(program, "x_axis");
  GLint y_axis_loc = glGetUniformLocation(program, "y_axis");
  GLint transform_loc = glGetUniformLocation(program, "transform");
  GLint projection_loc = glGetUniformLocation(program, "projection");

  assert(screen_loc != -1);
  assert(x_axis_loc != -1);
  assert(y_axis_loc != -1);
  assert(transform_loc != -1);
  assert(projection_loc != -1);

  GlfwContext ctx = {
    .program = program,
    .screen_loc = screen_loc,
    .x_axis_loc = x_axis_loc,
    .y_axis_loc = y_axis_loc,
    .transform_loc = transform_loc,
    .projection_loc = projection_loc,
  };

  glfwSetWindowUserPointer(window, &ctx);

  update_coordinate_system(&ctx);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);
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

      glBindVertexArray(quad_array);
      glUseProgram(program);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

      glfwSwapBuffers(window);
      glfwPollEvents();
    }

  glfwTerminate();
  exit(EXIT_SUCCESS);
}
