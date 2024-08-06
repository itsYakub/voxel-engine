#include "shader.h"

#include <stdlib.h>
#include <stdio.h>

#include "SDL2/SDL.h"

#include "core.h"

extern core_data CORE;

GLchar* LoadShaderCode(const GLchar* filepath) {
    FILE* shader_file = fopen(filepath, "rb");
    if(!shader_file) {
        fprintf(stderr, "[ERR] Could not open a file: %s\n", filepath);

        return NULL;
    }

    int shader_file_length = 0;

    fseek(shader_file, 0, SEEK_END);
    shader_file_length = ftell(shader_file);
    fseek(shader_file, 0, SEEK_SET);

    GLchar* result = (GLchar*) SDL_calloc(shader_file_length + 1, sizeof(GLchar*));
    if(!result) {
        fprintf(stderr, "[ERR] Could not allocate a dynamic char*\n");
        fclose(shader_file);

        return NULL;
    }

    fread(result, shader_file_length, 1, shader_file);
    result[shader_file_length] = '\0';

    fclose(shader_file);

    fprintf(stdout, "[INFO] SHADER: Shader code loaded successfully | Path: %s | Size: %i\n", filepath, shader_file_length);

    return result;
}

GLuint CreateShader(const GLchar* shader_code_filepath, GLuint shader_type) {
    const GLchar* shader_code =  LoadShaderCode(shader_code_filepath);
    GLuint result = glCreateShader(shader_type);

    glShaderSource(result, 1, &shader_code, NULL);

    glCompileShader(result);

    SDL_free((void*) shader_code);

    GLint compile_success;
    glGetShaderiv(result, GL_COMPILE_STATUS, &compile_success);
    if(compile_success != GL_TRUE) {
        GLchar buffer[1024];
        glGetShaderInfoLog(result, 1024, 0, buffer);
        fprintf(stderr, "[ERR] SHADER: %s\n", buffer);   
    }

    return result;
}

GLuint CreateProgram(GLuint vertex_shader, GLuint fragmnet_shader) {
    GLuint result = glCreateProgram();

    glAttachShader(result, vertex_shader);
    glAttachShader(result, fragmnet_shader);
    glLinkProgram(result);

    GLint link_success;
    glGetProgramiv(result, GL_LINK_STATUS, &link_success);
    if(link_success != GL_TRUE) {
        GLchar buffer[1024];
        glGetProgramInfoLog(result, 1024, 0, buffer);
        fprintf(stderr, "[ERR] PROGRAM: %s\n", buffer);   
    }

    DeleteShader(vertex_shader);
    DeleteShader(fragmnet_shader);

    return result;
}

GLuint* GetDefaultShader(GLuint shader_type) {
    switch(shader_type) {
        case GL_VERTEX_SHADER: return &CORE.shaders.shader_vertex_id;
        case GL_FRAGMENT_SHADER: return &CORE.shaders.shader_fragment_id;

        default: return NULL;
    }
}

GLuint* GetDefaultProgram() {
    return &CORE.shaders.shader_program_id;
}

void DeleteShader(GLuint shader) {
    glDeleteShader(shader);
}

void DeleteProgram(GLuint program) {
    glUseProgram(0);
    glDeleteProgram(program);
}

GLint GetShaderUniformLocation(GLuint program, const GLchar* uniform_name) {
    return glGetUniformLocation(program, uniform_name);
}