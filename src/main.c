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

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

void
framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  (void)window;
  glViewport(0, 0, width, height);
}

#define GRID_X_COUNT 8
#define GRID_Y_COUNT 6

#define GRID_QUAD_COUNT (GRID_X_COUNT * GRID_Y_COUNT)

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

  GLuint offset_buffer;
  glCreateBuffers(1, &offset_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, offset_buffer);
  glBufferData(GL_ARRAY_BUFFER, GRID_QUAD_COUNT * 2 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

  {
    float *offsets = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE);
    assert(offsets);

    for (size_t i = 0, k = 0; i < GRID_Y_COUNT; i++)
      {
        float y = i * 2.0 / GRID_Y_COUNT;
        for (size_t j = 0; j < GRID_X_COUNT; j++, k += 2)
          {
            float x = j * 2.0 / GRID_X_COUNT;
            offsets[k + 0] = x;
            offsets[k + 1] = y;
          }
      }

    assert(glUnmapBuffer(GL_ARRAY_BUFFER));
  }

  glBindVertexArray(quad_array);

  glBindBuffer(GL_ARRAY_BUFFER, quad_buffer);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, offset_buffer);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribDivisor(1, 1);

  GLuint program = 0;

  {
    GLuint vertex_shader = create_shader(GL_VERTEX_SHADER, "./shaders/grid.vert");
    GLuint fragment_shader = create_shader(GL_FRAGMENT_SHADER, "./shaders/default.frag");
    program = create_program(vertex_shader, fragment_shader);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
  }

  GLint transform_loc = glGetUniformLocation(program, "transform");
  assert(transform_loc != -1);

  {
    float matrix[3][2] = {
      { 0.9 * 2.0 / GRID_X_COUNT, 0 }, { 0, 0.9 * 2.0 / GRID_Y_COUNT }, { -1 + 0.05 * 2.0 / GRID_X_COUNT, -1 + 0.05 * 2.0 / GRID_Y_COUNT }, };

    glUseProgram(program);
    glUniformMatrix3x2fv(transform_loc, 1, GL_FALSE, (float *)matrix);
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.8, 0.0, 0.4, 1.0);

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
      glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, GRID_QUAD_COUNT);

      glfwSwapBuffers(window);
      glfwPollEvents();
    }

  glfwTerminate();
  exit(EXIT_SUCCESS);
}
