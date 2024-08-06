#if !defined (CAMERA_H)
#define CAMERA_H

#include <stdbool.h>

#include "glad/glad.h"
#include "cglm/types.h"

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

camera CameraInit(camera_mode mode, vec3 position, GLfloat field_of_view);
void CameraMatrix(camera* camera);
void CameraMovement(camera* camera, bool enable);

#endif // CAMERA_H