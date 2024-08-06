#if !defined (SHADER_H)
#define SHADER_H

#include "glad/glad.h"

GLchar* LoadShaderCode(const GLchar* filepath);

GLuint CreateShader(const GLchar* shader_code_filepath, GLuint shader_type);
GLuint CreateProgram(GLuint vertex_shader, GLuint fragmnet_shader);

GLuint* GetDefaultShader(GLuint shader_type);
GLuint* GetDefaultProgram();

void DeleteShader(GLuint shader);
void DeleteProgram(GLuint program);

GLint GetShaderUniformLocation(GLuint program, const GLchar* uniform_name);

#endif // SHADER_H