#if !defined (CORE_H)
#define CORE_H

#include "SDL2/SDL.h"
#include "glad/glad.h"
#include "cglm/types.h"

#include "camera.h"
#include "render_batch.h"

typedef struct {
    struct {
        SDL_Window* window;
        SDL_GLContext context; 

        ivec2 window_size;
        SDL_bool window_close;
    } window_context;

    struct {
        struct {
            SDL_bool key_state_current[SDL_NUM_SCANCODES];
            SDL_bool key_state_previous[SDL_NUM_SCANCODES];
        } keyboard;

        struct {
            ivec2 position;
            ivec2 position_previous;
            
            SDL_bool mouse_state_current[3];
            SDL_bool relative;
        } mouse;
    } input;

    struct {
        GLuint shader_vertex_id;
        GLuint shader_fragment_id;

        GLuint shader_program_id;
    } shaders;

    struct {
        GLuint vao_id;
        GLuint vbo_id;
        GLuint ebo_id;

        vert* vertices; GLuint vertices_count; GLuint vertices_count_max;
        GLuint* indices; GLuint indices_count; GLuint indices_count_max;
    } render_batch;

    struct {
        mat4 projection;    GLuint shader_loc_projection;
        mat4 view;          GLuint shader_loc_view;
        mat4 model;         GLuint shader_loc_model;
    } matrices;

} core_data;

void CreateWindow(ivec2 size, const GLchar* title);
void CloseWindow();

SDL_bool WindowCloseCallback();

void PollEvents();

void Clear(vec4 color);
void BeginRenderMode(camera* camera);
void EndRenderMode();

void DefaultMatrix();

#endif // CORE_H