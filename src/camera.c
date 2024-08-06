#include "camera.h"

#include "SDL2/SDL.h"
#include "cglm/cglm.h"

#include "input.h"
#include "shader.h"
#include "core.h"

extern core_data CORE;

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
    
    glUniformMatrix4fv(
        GetShaderUniformLocation(
            *GetDefaultProgram(), 
            "uMatrixProjection"
        ), 
        1, 
        GL_FALSE, 
        &camera->projection[0][0]
    );

    vec3 camera_center;
    glm_vec3_add(camera->position, camera->direction, camera_center);  

    glm_lookat(camera->position, camera_center, (vec3) { 0.0f, 1.0f, 0.0f }, camera->view);
    glUniformMatrix4fv(
        GetShaderUniformLocation(
            *GetDefaultProgram(), 
            "uMatrixView"
        ), 
        1, 
        GL_FALSE, 
        &camera->view[0][0]
    );
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
    }
}
