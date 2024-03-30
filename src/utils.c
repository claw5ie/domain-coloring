char *
read_entire_file(const char *filepath, size_t *file_size_loc)
{
  int fd = open(filepath, O_RDONLY);
  if (fd == -1)
    goto report_error;

  size_t size = 0;
  char *data = NULL;

  {
    struct stat stats;
    if (fstat(fd, &stats) == -1)
      goto report_error;
    size = stats.st_size;
  }

  data = malloc(size + 1);
  if (data == NULL)
    abort();
  else if (read(fd, data, size) == -1)
    goto report_error;
  data[size] = '\0';

  close(fd);

  if (file_size_loc != NULL)
    *file_size_loc = size;

  return data;

 report_error:
  perror(strerror(errno));
  exit(EXIT_FAILURE);
}

GLuint
create_shader(GLenum shader_type, const char *filepath)
{
  assert(shader_type == GL_VERTEX_SHADER
         || shader_type == GL_FRAGMENT_SHADER);

  GLuint shader = glCreateShader(shader_type);

  {
    size_t file_size = 0;
    char *file_data = read_entire_file(filepath, &file_size);
    GLint line_size = file_size;
    glShaderSource(shader, 1, (void *)&file_data, &line_size);
    free(file_data);
  }

  GLint status = 0;
  glCompileShader(shader);
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

  if (status != GL_TRUE)
    {
      GLint info_log_length = 0;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);

      const char *shader_type_as_string
        = shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment";
      if (info_log_length <= 0)
        fprintf(stderr,
                "ERROR: can't compile %s shader: no error message from OpenGL.\n",
                shader_type_as_string);
      else
        {
          char *error_message = malloc(info_log_length + 1);
          if (error_message == NULL)
            abort();
          glGetShaderInfoLog(shader, info_log_length, NULL, error_message);
          error_message[info_log_length] = '\0';
          fprintf(stderr,
                  "ERROR: can't compile %s shader:\n%s",
                  shader_type_as_string,
                  error_message);
          free(error_message);
        }

      glDeleteShader(shader);
      abort();
    }

  return shader;
}

GLuint
create_program(GLuint vertex_shader, GLuint fragment_shader)
{
  GLuint program = glCreateProgram();

  glAttachShader(program, vertex_shader);
  glAttachShader(program, fragment_shader);
  glLinkProgram(program);
  glDetachShader(program, vertex_shader);
  glDetachShader(program, fragment_shader);

  GLint status = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &status);

  if (status != GL_TRUE)
    {
      GLint info_log_length = 0;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);

      if (info_log_length <= 0)
        fputs("ERROR: can't link program: no error message from OpenGL.\n",
              stderr);
      else
        {
          char *error_message = malloc(info_log_length + 1);
          if (error_message == NULL)
            abort();
          glGetProgramInfoLog(program, info_log_length, NULL, error_message);
          error_message[info_log_length] = '\0';
          fprintf(stderr,
                  "ERROR: can't link program:\n%s",
                  error_message);
          free(error_message);
        }

      glDeleteProgram(program);
      abort();
    }

  return program;
}
