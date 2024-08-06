#include "core.h"

#include "SDL2/SDL.h"
#include "cglm/cglm.h"

#include "render_batch.h"
#include "input.h" 

core_data CORE = { 0 };

void CreateWindow(ivec2 size, const GLchar* title) {
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "[ERR] %s\n", SDL_GetError());

        return;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    CORE.window_context.window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, size[0], size[1], SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    SDL_GetWindowSize(
        CORE.window_context.window,
        &CORE.window_context.window_size[0],
        &CORE.window_context.window_size[1]
    );

    CORE.window_context.window_close = SDL_FALSE;

    CORE.window_context.context = SDL_GL_CreateContext(CORE.window_context.window);
    SDL_GL_MakeCurrent(CORE.window_context.window, CORE.window_context.context);
    gladLoadGL();

    printf("[INFO] SDL: Version: %i.%i.%i\n", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);
    printf("[INFO] OPENGL: Version: %s\n", glGetString(GL_VERSION));

    printf("[INFO] WINDOW: Successfully created an SDL Window | Title: %s | Size: x.%i y.%i\n", title, size[0], size[1]);
}

void CloseWindow() {
    printf("[INFO] OPENGL: Closing an OpenGL context\n");
    SDL_GL_DeleteContext(CORE.window_context.context);

    printf("[INFO] WINDOW: Closing an SDL Window\n");
    SDL_DestroyWindow(CORE.window_context.window);

    printf("[INFO] SDL: Closing an SDL Platform\n");
    SDL_Quit();
}

SDL_bool WindowCloseCallback() {
    return CORE.window_context.window_close;
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

void Clear(vec4 color) {
    glClearColor(color[0], color[1], color[2], color[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void BeginRenderMode(camera* camera) {
    glViewport(0, 0, CORE.window_context.window_size[0], CORE.window_context.window_size[1]);

    if(camera != NULL) {
        CameraMatrix(camera);
    } else {
        DefaultMatrix();
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
    
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);
}

void EndRenderMode() {
    DrawRenderBatch();

    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);

    SDL_GL_SwapWindow(CORE.window_context.window);
    PollEvents();
}

void DefaultMatrix() {
    glm_mat4_identity(CORE.matrices.projection);
    glm_mat4_identity(CORE.matrices.view);

    glm_perspective(glm_rad(45.0f), (float) (CORE.window_context.window_size[0]) / (float) (CORE.window_context.window_size[1]), 0.001f, 16384.0f, CORE.matrices.projection);
    glUniformMatrix4fv(CORE.matrices.shader_loc_projection, 1, GL_FALSE, &CORE.matrices.projection[0][0]);

    glm_translate(CORE.matrices.view, (vec3) { 0 });        
    glUniformMatrix4fv(CORE.matrices.shader_loc_view, 1, GL_FALSE, &CORE.matrices.view[0][0]);
}
