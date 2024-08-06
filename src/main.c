#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "SDL_events.h"
#include "SDL_mouse.h"
#include "SDL_scancode.h"
#include "SDL_stdinc.h"
#include "SDL_video.h"
#include "cglm/cglm.h"
#include "cglm/affine-pre.h"
#include "cglm/cam.h"
#include "cglm/ivec3.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/util.h"
#include "cglm/vec3.h"
#include "glad/glad.h"
#include "SDL.h"

#include <GL/gl.h>  

#define VOXEL_SIZE 16.0f

typedef enum {
    CAMERA_PERSPECTIVE,
    CAMERA_ORTHOGRAPHIC
} camera_mode;

typedef struct {
    camera_mode mode;

    vec3 position;

    GLfloat field_of_view;
    GLfloat plane_near; // How close you can see the world
    GLfloat plane_far; // How far you can see the world

    // Camera's direction
    vec3 direction;
    GLfloat pitch; // Rotation on X axis (Up - Down)
    GLfloat yaw; // Rotation on Y axis (Left - Right)
    GLfloat sensitivity;

    // View-Projection matrices, later-on pushed to the shader
    mat4 projection;
    mat4 view;
} camera;

typedef struct {
    vec3 position;
    vec4 color;
    vec2 texcoord;
    GLint texid;
} vert;

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

core_data CORE;

void PollEvents();

SDL_bool GetKeyDown(SDL_Scancode code);
SDL_bool GetKeyUp(SDL_Scancode code);

int GetMouseX();
int GetMouseY();
int GetMouseDeltaX();
int GetMouseDeltaY();
SDL_bool GetButtonDown(int mouse_button);
SDL_bool GetButtonUp(int mouse_button);

camera CameraInit(camera_mode mode, vec3 position, GLfloat field_of_view);
void CameraMatrix(camera* camera);
void CameraMovement(camera* camera, bool enable);

void DefaultMatrix();

GLchar* LoadShaderCode(const GLchar* filepath);

void LoadRenderBatch(const GLuint triangles_count);
void UnloadRenderBatch();
void DrawRenderBatch();
void PushRenderBatchVertexData(vec3* positions, vec4* colors, vec2* texcoord, GLint texid, GLuint data_count);
void PushRenderBatchIndexData(GLuint* index_data, GLuint data_count);

void DrawQuad(vec3 point_a, vec3 point_b, vec4 color);
void RenderVoxel(vec3 position, int size, vec4 tint, bool draw_top, bool draw_down, bool draw_front, bool draw_back, bool draw_left, bool draw_right);

int main(int argc, const char* argv[]) {
    camera camera = CameraInit(CAMERA_PERSPECTIVE, (vec3) { 0.0f, 0.0f, 0.0f }, 60.0f);

    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "[ERR] %s\n", SDL_GetError());

        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    CORE.window_context.window = SDL_CreateWindow("Voxel Engine 1.0", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 640, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    SDL_GetWindowSize(
        CORE.window_context.window,
        &CORE.window_context.window_size[0],
        &CORE.window_context.window_size[1]
    );

    CORE.window_context.window_close = SDL_FALSE;

    CORE.window_context.context = SDL_GL_CreateContext(CORE.window_context.window);
    SDL_GL_MakeCurrent(CORE.window_context.window, CORE.window_context.context);
    gladLoadGL();

    // Creating the basic shaders

    const GLchar* shader_vertex_code = LoadShaderCode("../res/shaders/vertex.glsl");
    const GLchar* shader_fragment_code = LoadShaderCode("../res/shaders/fragment.glsl");

    CORE.shaders.shader_program_id = glCreateProgram();
    CORE.shaders.shader_vertex_id = glCreateShader(GL_VERTEX_SHADER);
    CORE.shaders.shader_fragment_id = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(CORE.shaders.shader_vertex_id, 1, &shader_vertex_code, NULL);
    glShaderSource(CORE.shaders.shader_fragment_id, 1, &shader_fragment_code, NULL);

    glCompileShader(CORE.shaders.shader_vertex_id);
    glCompileShader(CORE.shaders.shader_fragment_id);

    GLint compile_success;
    glGetShaderiv(CORE.shaders.shader_vertex_id, GL_COMPILE_STATUS, &compile_success);
    if(compile_success != GL_TRUE) {
        GLchar buffer[1024];
        glGetShaderInfoLog(CORE.shaders.shader_vertex_id, 1024, 0, buffer);
        fprintf(stderr, "[ERR] SHADER: %s\n", buffer);   
    }

    glGetShaderiv(CORE.shaders.shader_fragment_id, GL_COMPILE_STATUS, &compile_success);
    if(compile_success != GL_TRUE) {
        GLchar buffer[1024];
        glGetShaderInfoLog(CORE.shaders.shader_fragment_id, 1024, 0, buffer);
        fprintf(stderr, "[ERR] SHADER: %s\n", buffer);   
    }

    glAttachShader(CORE.shaders.shader_program_id, CORE.shaders.shader_vertex_id);
    glAttachShader(CORE.shaders.shader_program_id, CORE.shaders.shader_fragment_id);
    glLinkProgram(CORE.shaders.shader_program_id);

    GLint link_success;
    glGetProgramiv(CORE.shaders.shader_program_id, GL_LINK_STATUS, &link_success);
    if(link_success != GL_TRUE) {
        GLchar buffer[1024];
        glGetProgramInfoLog(CORE.shaders.shader_program_id, 1024, 0, buffer);
        fprintf(stderr, "[ERR] PROGRAM: %s\n", buffer);   
    }

    glDeleteShader(CORE.shaders.shader_vertex_id);
    glDeleteShader(CORE.shaders.shader_fragment_id);

    SDL_free((void*) shader_vertex_code);
    SDL_free((void*) shader_fragment_code);

    glUseProgram(CORE.shaders.shader_program_id);

    // Render-batch

    LoadRenderBatch(1024);

    // View-Projection matrices
    CORE.matrices.shader_loc_projection =   glGetUniformLocation(CORE.shaders.shader_program_id, "uMatrixProjection");
    CORE.matrices.shader_loc_view =         glGetUniformLocation(CORE.shaders.shader_program_id, "uMatrixView");

    int rotation = 0;
    ivec3 camera_position = { 0 };

    while(!CORE.window_context.window_close) {  
        CameraMovement(&camera, true);   
        CameraMatrix(&camera);

        glViewport(0, 0, CORE.window_context.window_size[0], CORE.window_context.window_size[1]);

        glClearColor(0.1f, 0.1, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        glFrontFace(GL_CCW);

        RenderVoxel((vec3) { 0.0f * VOXEL_SIZE, 0.0f, 0.0f * VOXEL_SIZE }, VOXEL_SIZE, (vec4) { 1.0f, 1.0f, 1.0f, 1.0f }, true, true, false, true, true, false);
        RenderVoxel((vec3) { 0.0f * VOXEL_SIZE, 0.0f, 1.0f * VOXEL_SIZE }, VOXEL_SIZE, (vec4) { 1.0f, 1.0f, 1.0f, 1.0f }, true, true, true, false, true, false);
        RenderVoxel((vec3) { 1.0f * VOXEL_SIZE, 0.0f, 0.0f * VOXEL_SIZE }, VOXEL_SIZE, (vec4) { 1.0f, 1.0f, 1.0f, 1.0f }, true, true, false, true, false, true);
        RenderVoxel((vec3) { 1.0f * VOXEL_SIZE, 0.0f, 1.0f * VOXEL_SIZE }, VOXEL_SIZE, (vec4) { 1.0f, 1.0f, 1.0f, 1.0f }, true, true, true, false, false, true);

        DrawRenderBatch();

        SDL_GL_SwapWindow(CORE.window_context.window);
        PollEvents();
    }

    UnloadRenderBatch();

    glUseProgram(0);
    glDeleteProgram(CORE.shaders.shader_program_id);

    SDL_GL_DeleteContext(CORE.window_context.context);
    SDL_DestroyWindow(CORE.window_context.window);

    SDL_Quit();

    return 0;
}

void PollEvents() {
    if(CORE.input.mouse.relative) {
        CORE.input.mouse.position[0] = CORE.window_context.window_size[0] / 2;
        CORE.input.mouse.position[1] = CORE.window_context.window_size[1] / 2;
    } 

    CORE.input.mouse.position_previous[0] = CORE.input.mouse.position[0];
    CORE.input.mouse.position_previous[1] = CORE.input.mouse.position[1];

    SDL_Event sdl_event;
    while(SDL_PollEvent(&sdl_event)) {
        switch(sdl_event.type) {
            case SDL_QUIT: {
                CORE.window_context.window_close = SDL_TRUE;
            } break;

            case SDL_WINDOWEVENT: {
                switch(sdl_event.window.event) {
                    case SDL_WINDOWEVENT_RESIZED: {
                        SDL_GetWindowSize(
                            CORE.window_context.window,
                            &CORE.window_context.window_size[0],
                            &CORE.window_context.window_size[1]
                        );
                    } break;
                }
            } break;

            // TODO:
            // Implement keyboard inputing

            case SDL_KEYDOWN: {
                CORE.input.keyboard.key_state_current[sdl_event.key.keysym.scancode] = SDL_TRUE;

                if(GetKeyDown(SDL_SCANCODE_ESCAPE)) {
                    CORE.window_context.window_close = SDL_TRUE;
                }
            } break;

            case SDL_KEYUP: {
                CORE.input.keyboard.key_state_current[sdl_event.key.keysym.scancode] = SDL_FALSE;
            } break;

            case SDL_MOUSEBUTTONDOWN: {
                int button = sdl_event.button.button;
                CORE.input.mouse.mouse_state_current[button] = SDL_TRUE;
            } break;

            case SDL_MOUSEBUTTONUP: {
                int button = sdl_event.button.button;
                CORE.input.mouse.mouse_state_current[button] = SDL_FALSE;
            } break;

            case SDL_MOUSEMOTION: {
                if(!CORE.input.mouse.relative) {                    
                    CORE.input.mouse.position[0] = sdl_event.motion.xrel;
                    CORE.input.mouse.position[1] = sdl_event.motion.yrel;

                    CORE.input.mouse.position_previous[0] = CORE.window_context.window_size[0] / 2;
                    CORE.input.mouse.position_previous[1] = CORE.window_context.window_size[1] / 2;
                } else {
                    CORE.input.mouse.position[0] = sdl_event.motion.x;
                    CORE.input.mouse.position[1] = sdl_event.motion.y;
                }
            } break;
        }
    }
}

SDL_bool GetKeyDown(SDL_Scancode code) {
    return CORE.input.keyboard.key_state_current[code];
}

SDL_bool GetKeyUp(SDL_Scancode code) {
    return !CORE.input.keyboard.key_state_current[code];
}

int GetMouseX() {
    return CORE.input.mouse.position[0];
}

int GetMouseY() {
    return CORE.input.mouse.position[1];
}

int GetMouseDeltaX() {
    return CORE.input.mouse.position[0] - CORE.input.mouse.position_previous[0];
}

int GetMouseDeltaY() {
    return CORE.input.mouse.position[1] - CORE.input.mouse.position_previous[1];
}

SDL_bool GetButtonDown(int mouse_button) {
    return CORE.input.mouse.mouse_state_current[mouse_button];
}

SDL_bool GetButtonUp(int mouse_button) {
    return !CORE.input.mouse.mouse_state_current[mouse_button];
}

camera CameraInit(camera_mode mode, vec3 position, GLfloat field_of_view) {
    camera result = { 0 };

    result.mode = mode;

    result.position[0] = position[0];
    result.position[1] = position[1];
    result.position[2] = position[2];

    result.direction[0] = 0.0f;
    result.direction[1] = 0.0f;
    result.direction[2] = -1.0f;

    glm_mat4_identity(result.projection);
    glm_mat4_identity(result.view);

    result.field_of_view = field_of_view;
    result.plane_near = 0.001f;
    result.plane_far = 16384.0f;

    result.pitch = 0.0f;
    result.yaw = -90.0f;

    result.sensitivity = 0.1f;

    return result;
}

void CameraMatrix(camera* camera) {
    glm_mat4_identity(camera->projection);
    glm_mat4_identity(camera->view);

    switch(camera->mode) {
        case CAMERA_PERSPECTIVE: {
            glm_perspective(glm_rad(camera->field_of_view), (float) (CORE.window_context.window_size[0]) / (float) (CORE.window_context.window_size[1]), camera->plane_near, camera->plane_far, camera->projection);
        } break;

        case CAMERA_ORTHOGRAPHIC: {
            glm_ortho(0.0f, CORE.window_context.window_size[0], 0.0f, CORE.window_context.window_size[1], camera->plane_near, camera->plane_far, camera->projection);
        }
    }
    
    glUniformMatrix4fv(CORE.matrices.shader_loc_projection, 1, GL_FALSE, &camera->projection[0][0]);

    //glm_translate(camera->view, camera->position);  

    vec3 camera_center;
    glm_vec3_add(camera->position, camera->direction, camera_center);  

    glm_lookat(camera->position, camera_center, (vec3) { 0.0f, 1.0f, 0.0f }, camera->view);
    glUniformMatrix4fv(CORE.matrices.shader_loc_view, 1, GL_FALSE, &camera->view[0][0]);
}

void CameraMovement(camera* camera, bool enable) {
    if(!enable) {
        return;
    }

    { // Keyboard movement
        if(GetKeyDown(SDL_SCANCODE_W)) {
            camera->position[0] += 1.0f * camera->direction[0];
            camera->position[1] += 1.0f * camera->direction[1];
            camera->position[2] += 1.0f * camera->direction[2];
        } if(GetKeyDown(SDL_SCANCODE_S)) {
            camera->position[0] -= 1.0f * camera->direction[0];
            camera->position[1] -= 1.0f * camera->direction[1];
            camera->position[2] -= 1.0f * camera->direction[2];
        }

        if(GetKeyDown(SDL_SCANCODE_A)) {
            vec3 orientation_normalized;
            glm_cross(camera->direction, (vec3) { 0.0f, 1.0f, 0.0f }, orientation_normalized);
            glm_normalize(orientation_normalized);

            camera->position[0] -= 1.0f * orientation_normalized[0];
            camera->position[1] -= 1.0f * orientation_normalized[1];
            camera->position[2] -= 1.0f * orientation_normalized[2];
        } if(GetKeyDown(SDL_SCANCODE_D)) {
            vec3 orientation_normalized;
            glm_cross(camera->direction, (vec3) { 0.0f, 1.0f, 0.0f }, orientation_normalized);
            glm_normalize(orientation_normalized);

            camera->position[0] += 1.0f * orientation_normalized[0];
            camera->position[1] += 1.0f * orientation_normalized[1];
            camera->position[2] += 1.0f * orientation_normalized[2];
        }

        if(GetKeyDown(SDL_SCANCODE_SPACE)) {
            vec3 vector_up = { 0.0f, 1.0f, 0.0f };

            camera->position[0] += 1.0f * vector_up[0];
            camera->position[1] += 1.0f * vector_up[1];
            camera->position[2] += 1.0f * vector_up[2];
        } if(GetKeyDown(SDL_SCANCODE_LSHIFT)) {
            vec3 vector_up = { 0.0f, 1.0f, 0.0f };

            camera->position[0] -= 1.0f * vector_up[0];
            camera->position[1] -= 1.0f * vector_up[1];
            camera->position[2] -= 1.0f * vector_up[2];
        }
    }

    { // Mouse movement
        SDL_SetRelativeMouseMode(SDL_TRUE);
        CORE.input.mouse.relative = SDL_TRUE;
        SDL_WarpMouseInWindow(CORE.window_context.window, CORE.window_context.window_size[0] / 2, CORE.window_context.window_size[1] / 2);
        
        camera->pitch -= camera->sensitivity * GetMouseDeltaY(); // Horizontal rotation (On the X axis) (up-down)
        camera->pitch = glm_clamp(camera->pitch, -80.0f, 80.0f);

        camera->yaw += camera->sensitivity * GetMouseDeltaX(); // Vertical rotation (On the Y axis) (left-right)

        vec3 direction;
        direction[0] = SDL_cos(glm_rad(camera->yaw)) * SDL_cos(glm_rad(camera->pitch));
        direction[1] = SDL_sin(glm_rad(camera->pitch));
        direction[2] = SDL_sin(glm_rad(camera->yaw)) * SDL_cos(glm_rad(camera->pitch));

        glm_vec3_normalize_to(direction, camera->direction);

        // TODO: 
        // Something causes the camera to slightly move around the scene when there isn't any motion (or is there?)
    }
}

void DefaultMatrix() {
    glm_mat4_identity(CORE.matrices.projection);
    glm_mat4_identity(CORE.matrices.view);

    glm_perspective(glm_rad(45.0f), (float) (CORE.window_context.window_size[0]) / (float) (CORE.window_context.window_size[1]), 0.001f, 16384.0f, CORE.matrices.projection);
    glUniformMatrix4fv(CORE.matrices.shader_loc_projection, 1, GL_FALSE, &CORE.matrices.projection[0][0]);

    glm_translate(CORE.matrices.view, (vec3) { 0 });        
    glUniformMatrix4fv(CORE.matrices.shader_loc_view, 1, GL_FALSE, &CORE.matrices.view[0][0]);
}

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

    result[shader_file_length] = '\0';
    fread(result, shader_file_length, 1, shader_file);

    fclose(shader_file);

    fprintf(stdout, "[INFO] SHADER: Shader code loaded successfully | Path: %s | Size: %i\n", filepath, shader_file_length);

    return result;
}

void LoadRenderBatch(const GLuint triangles_count) {
    glGenVertexArrays(1, &CORE.render_batch.vao_id);
    glGenBuffers(1, &CORE.render_batch.vbo_id);
    glGenBuffers(1, &CORE.render_batch.ebo_id);

    CORE.render_batch.vertices_count_max = triangles_count * 3;
    CORE.render_batch.indices_count_max = triangles_count * 3;

    CORE.render_batch.vertices = (vert*) SDL_calloc(CORE.render_batch.vertices_count_max, sizeof(vert));
    CORE.render_batch.indices = (GLuint*) SDL_calloc(CORE.render_batch.indices_count_max, sizeof(GLuint));

    CORE.render_batch.vertices_count = 0;
    CORE.render_batch.indices_count = 0;
}

void UnloadRenderBatch() {
    SDL_free(CORE.render_batch.vertices);
    SDL_free(CORE.render_batch.indices);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDeleteBuffers(1, &CORE.render_batch.vbo_id);
    glDeleteBuffers(1, &CORE.render_batch.ebo_id);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &CORE.render_batch.vao_id);
}

void DrawRenderBatch() {
    glBindVertexArray(CORE.render_batch.vao_id);

    const GLuint vertices_stride = 3 /* X, Y, Z */ + 4 /* R, G, B, A */ + 2 /* U, V */ + 1 /* ID */;
    GLfloat vertices[CORE.render_batch.vertices_count * vertices_stride];

    for(int data_index = 0, vert_index = 0; data_index < CORE.render_batch.vertices_count * vertices_stride; vert_index++) {
        vertices[data_index++] = CORE.render_batch.vertices[vert_index].position[0];
        vertices[data_index++] = CORE.render_batch.vertices[vert_index].position[1];
        vertices[data_index++] = CORE.render_batch.vertices[vert_index].position[2];

        vertices[data_index++] = CORE.render_batch.vertices[vert_index].color[0];
        vertices[data_index++] = CORE.render_batch.vertices[vert_index].color[1];
        vertices[data_index++] = CORE.render_batch.vertices[vert_index].color[2];
        vertices[data_index++] = CORE.render_batch.vertices[vert_index].color[3];

        vertices[data_index++] = CORE.render_batch.vertices[vert_index].texcoord[0];
        vertices[data_index++] = CORE.render_batch.vertices[vert_index].texcoord[1];

        vertices[data_index++] = CORE.render_batch.vertices[vert_index].texid;
    }

    glBindBuffer(GL_ARRAY_BUFFER, CORE.render_batch.vbo_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CORE.render_batch.ebo_id);
    
    glBufferData(GL_ARRAY_BUFFER, CORE.render_batch.vertices_count * vertices_stride * sizeof(GLfloat), vertices, GL_DYNAMIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, CORE.render_batch.indices_count * sizeof(GLuint), CORE.render_batch.indices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, vertices_stride * sizeof(GLfloat), (void*) ((0) * sizeof(GLfloat)));
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, vertices_stride * sizeof(GLfloat), (void*) ((0 + 3) * sizeof(GLfloat)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vertices_stride * sizeof(GLfloat), (void*) ((0 + 3 + 4) * sizeof(GLfloat)));
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, vertices_stride * sizeof(GLfloat), (void*) ((0 + 3 + 4 + 2) * sizeof(GLfloat)));

    glDrawElements(GL_TRIANGLES, CORE.render_batch.indices_count, GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    CORE.render_batch.vertices_count = 0;
    CORE.render_batch.indices_count = 0;
}

void PushRenderBatchVertexData(vec3* positions, vec4* colors, vec2* texcoord, GLint texid, GLuint data_count) {
    for(int vert_index = 0; vert_index < data_count; vert_index++) {
        CORE.render_batch.vertices[CORE.render_batch.vertices_count++] = (vert) {
            .position[0] = positions[vert_index][0],
            .position[1] = positions[vert_index][1],
            .position[2] = positions[vert_index][2],

            .color[0] = colors[vert_index][0],
            .color[1] = colors[vert_index][1],
            .color[2] = colors[vert_index][2],
            .color[3] = colors[vert_index][3],

            .texcoord[0] = texcoord[vert_index][0],
            .texcoord[1] = texcoord[vert_index][1],
            
            .texid = texid
        };
    }
}

void PushRenderBatchIndexData(GLuint* index_data, GLuint data_count) {
    int largest_indicy = 0;
    for(int indices = 0; indices < CORE.render_batch.indices_count; indices++) {
        if(largest_indicy < CORE.render_batch.indices[indices]) {
            largest_indicy = CORE.render_batch.indices[indices];
        }
    }

    int indicy_base = largest_indicy + (largest_indicy <= 0 ? 0 : 1);
    for(int indices = 0; indices < data_count; indices++) {
        CORE.render_batch.indices[CORE.render_batch.indices_count++] = indicy_base + index_data[indices];
    }
}

void DrawQuad(vec3 point_a, vec3 point_b, vec4 color) {
    // Quad data
    vec3 vertex_positions[] = {
        { point_a[0], point_a[1], point_a[2] },
        { point_b[0], point_a[1], point_b[2] },
        { point_b[0], point_b[1], point_b[2] },
        { point_a[0], point_b[1], point_a[2] },
    };

    vec4 vertex_color[] = {
        { 0.8f, 0.2f, 0.2f, 1.0f },
        { 0.2f, 0.8f, 0.2f, 1.0f },
        { 0.2f, 0.2f, 0.8f, 1.0f },
        { 0.8f, 0.8f, 0.8f, 1.0f },
    };

    vec2 vertex_texcoord[] = {
        { 0.0f, 0.0f },
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f },
    };

    GLuint index_data[] = {
        0, 1, 2,
        0, 2, 3
    };

    for(int vert_index = 0; vert_index < 4; vert_index++) {
        CORE.render_batch.vertices[CORE.render_batch.vertices_count++] = (vert) {
            .position[0] = vertex_positions[vert_index][0],
            .position[1] = vertex_positions[vert_index][1],
            .position[2] = vertex_positions[vert_index][2],

            .color[0] = vertex_color[vert_index][0],
            .color[1] = vertex_color[vert_index][1],
            .color[2] = vertex_color[vert_index][2],
            .color[3] = vertex_color[vert_index][3],

            .texcoord[0] = vertex_texcoord[vert_index][0],
            .texcoord[1] = vertex_texcoord[vert_index][1],
            
            .texid = 0.0f
        };
    }

    int largest_indicy = 0;
    for(int indices = 0; indices < CORE.render_batch.indices_count; indices++) {
        if(largest_indicy < CORE.render_batch.indices[indices]) {
            largest_indicy = CORE.render_batch.indices[indices];
        }
    }

    int indicy_base = largest_indicy + (largest_indicy <= 0 ? 0 : 1);
    for(int indices = 0; indices < 6; indices++) {
        CORE.render_batch.indices[CORE.render_batch.indices_count++] = indicy_base + index_data[indices];
    }
}

void RenderVoxel(vec3 position, int size, vec4 tint, bool draw_top, bool draw_down, bool draw_front, bool draw_back, bool draw_left, bool draw_right) {
    // TODO:
    // Simplify this function
    // (Maybe create a list of indices for each 6 faces of the voxel, and then iterate through it with a simple for-loop)

    //
    //       2 ---- 3
    //      /|     /|
    //     0 ---- 1 |
    //     | 6 ---| 7
    //     |/     |/
    //     4 ---- 5
    //

    vec3 vertex_positions[] = {
        { position[0],          position[1],        position[2] },          // 0,0,0
        { position[0] + size,   position[1],        position[2] },          // 1,0,0
        { position[0],          position[1],        position[2] + size },   // 0,0,1
        { position[0] + size,   position[1],        position[2] + size },   // 1,0,1
        { position[0],          position[1] - size, position[2] },          // 0,1,0
        { position[0] + size,   position[1] - size, position[2] },          // 1,1,0
        { position[0],          position[1] - size, position[2] + size },   // 0,1,1
        { position[0] + size,   position[1] - size, position[2] + size },   // 1,1,1
    };

    GLuint index_data[] = {
        0, 1, 2,
        3, 2, 1
    };

    GLuint index_data_reverse[] = {
        2, 1, 0,
        1, 2, 3
    };

    if(draw_top) { // Face: UP
        vec3 face_vertices[4] = {
            { vertex_positions[0][0], vertex_positions[0][1], vertex_positions[0][2] },
            { vertex_positions[1][0], vertex_positions[1][1], vertex_positions[1][2] },
            { vertex_positions[2][0], vertex_positions[2][1], vertex_positions[2][2] },
            { vertex_positions[3][0], vertex_positions[3][1], vertex_positions[3][2] },
        };

        GLfloat factor = 1.0f;

        vec4 face_color[] = {
            { tint[0] * factor, tint[1] * factor, tint[2] * factor, tint[3] }, // 0,0,0
            { tint[0] * factor, tint[1] * factor, tint[2] * factor, tint[3] }, // 1,0,0
            { tint[0] * factor, tint[1] * factor, tint[2] * factor, tint[3] }, // 0,0,1
            { tint[0] * factor, tint[1] * factor, tint[2] * factor, tint[3] }, // 1,0,1
        };

        vec2 vertex_texcoord[] = {
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 0.0f, 1.0f },
            { 1.0f, 1.0f },
        };

        PushRenderBatchVertexData(face_vertices, face_color, vertex_texcoord, 0, 4);
        PushRenderBatchIndexData(index_data, 6);
    }

    if(draw_down) { // Face: DOWN
        vec3 face_vertices[4] = {
            { vertex_positions[4][0], vertex_positions[4][1], vertex_positions[4][2] },
            { vertex_positions[5][0], vertex_positions[5][1], vertex_positions[5][2] },
            { vertex_positions[6][0], vertex_positions[6][1], vertex_positions[6][2] },
            { vertex_positions[7][0], vertex_positions[7][1], vertex_positions[7][2] },
        };

        GLfloat factor = 0.7f;

        vec4 face_color[] = {
            { tint[0] * factor, tint[1] * factor, tint[2] * factor, tint[3] }, // 0,0,0
            { tint[0] * factor, tint[1] * factor, tint[2] * factor, tint[3] }, // 1,0,0
            { tint[0] * factor, tint[1] * factor, tint[2] * factor, tint[3] }, // 0,0,1
            { tint[0] * factor, tint[1] * factor, tint[2] * factor, tint[3] }, // 1,0,1
        };

        vec2 vertex_texcoord[] = {
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 0.0f, 1.0f },
            { 1.0f, 1.0f },
        };

        PushRenderBatchVertexData(face_vertices, face_color, vertex_texcoord, 0, 4);
        PushRenderBatchIndexData(index_data_reverse, 6);
    }

    if(draw_front) { // Face: FRONT
        vec3 face_vertices[4] = {
            { vertex_positions[2][0], vertex_positions[2][1], vertex_positions[2][2] },
            { vertex_positions[3][0], vertex_positions[3][1], vertex_positions[3][2] },
            { vertex_positions[6][0], vertex_positions[6][1], vertex_positions[6][2] },
            { vertex_positions[7][0], vertex_positions[7][1], vertex_positions[7][2] },
        };

        GLfloat factor = 0.80f;

        vec4 face_color[] = {
            { tint[0] * factor, tint[1] * factor, tint[2] * factor, tint[3] }, // 0,0,0
            { tint[0] * factor, tint[1] * factor, tint[2] * factor, tint[3] }, // 1,0,0
            { tint[0] * factor, tint[1] * factor, tint[2] * factor, tint[3] }, // 0,0,1
            { tint[0] * factor, tint[1] * factor, tint[2] * factor, tint[3] }, // 1,0,1
        };

        vec2 vertex_texcoord[] = {
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 0.0f, 1.0f },
            { 1.0f, 1.0f },
        };

        PushRenderBatchVertexData(face_vertices, face_color, vertex_texcoord, 0, 4);
        PushRenderBatchIndexData(index_data, 6);
    }

    if(draw_back) { // Face: BACK
        vec3 face_vertices[4] = {
            { vertex_positions[0][0], vertex_positions[0][1], vertex_positions[0][2] },
            { vertex_positions[1][0], vertex_positions[1][1], vertex_positions[1][2] },
            { vertex_positions[4][0], vertex_positions[4][1], vertex_positions[4][2] },
            { vertex_positions[5][0], vertex_positions[5][1], vertex_positions[5][2] },
        };

        GLfloat factor = 0.80f;

        vec4 face_color[] = {
            { tint[0] * factor, tint[1] * factor, tint[2] * factor, tint[3] }, // 0,0,0
            { tint[0] * factor, tint[1] * factor, tint[2] * factor, tint[3] }, // 1,0,0
            { tint[0] * factor, tint[1] * factor, tint[2] * factor, tint[3] }, // 0,0,1
            { tint[0] * factor, tint[1] * factor, tint[2] * factor, tint[3] }, // 1,0,1
        };

        vec2 vertex_texcoord[] = {
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 0.0f, 1.0f },
            { 1.0f, 1.0f },
        };

        PushRenderBatchVertexData(face_vertices, face_color, vertex_texcoord, 0, 4);
        PushRenderBatchIndexData(index_data_reverse, 6);
    }

    if(draw_left) { // Face: LEFT
        vec3 face_vertices[4] = {
            { vertex_positions[0][0], vertex_positions[0][1], vertex_positions[0][2] },
            { vertex_positions[2][0], vertex_positions[2][1], vertex_positions[2][2] },
            { vertex_positions[4][0], vertex_positions[4][1], vertex_positions[4][2] },
            { vertex_positions[6][0], vertex_positions[6][1], vertex_positions[6][2] },
        };

        GLfloat factor = 0.86f;

        vec4 face_color[] = {
            { tint[0] * factor, tint[1] * factor, tint[2] * factor, tint[3] }, // 0,0,0
            { tint[0] * factor, tint[1] * factor, tint[2] * factor, tint[3] }, // 1,0,0
            { tint[0] * factor, tint[1] * factor, tint[2] * factor, tint[3] }, // 0,0,1
            { tint[0] * factor, tint[1] * factor, tint[2] * factor, tint[3] }, // 1,0,1
        };

        vec2 vertex_texcoord[] = {
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 0.0f, 1.0f },
            { 1.0f, 1.0f },
        };

        PushRenderBatchVertexData(face_vertices, face_color, vertex_texcoord, 0, 4);
        PushRenderBatchIndexData(index_data, 6);
    }

    if(draw_right) { // Face: RIGHT
        vec3 face_vertices[4] = {
            { vertex_positions[1][0], vertex_positions[1][1], vertex_positions[1][2] },
            { vertex_positions[3][0], vertex_positions[3][1], vertex_positions[3][2] },
            { vertex_positions[5][0], vertex_positions[5][1], vertex_positions[5][2] },
            { vertex_positions[7][0], vertex_positions[7][1], vertex_positions[7][2] },
        };

        GLfloat factor = 0.86f;

        vec4 face_color[] = {
            { tint[0] * factor, tint[1] * factor, tint[2] * factor, tint[3] }, // 0,0,0
            { tint[0] * factor, tint[1] * factor, tint[2] * factor, tint[3] }, // 1,0,0
            { tint[0] * factor, tint[1] * factor, tint[2] * factor, tint[3] }, // 0,0,1
            { tint[0] * factor, tint[1] * factor, tint[2] * factor, tint[3] }, // 1,0,1
        };

        vec2 vertex_texcoord[] = {
            { 0.0f, 0.0f },
            { 1.0f, 0.0f },
            { 0.0f, 1.0f },
            { 1.0f, 1.0f },
        };

        PushRenderBatchVertexData(face_vertices, face_color, vertex_texcoord, 0, 4);
        PushRenderBatchIndexData(index_data_reverse, 6);
    }
}