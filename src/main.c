#include <stdio.h>
#include <stdlib.h>

#include "glad/glad.h"

#include "core.h"
#include "input.h"
#include "camera.h"
#include "render_batch.h"
#include "shader.h"
#include "voxel.h"

#include <GL/gl.h>  

#define VOXEL_SIZE 16.0f

int main(int argc, const char* argv[]) {
    CreateWindow((ivec2) { 640, 640 }, "Voxel Engine 1.0");

    camera camera = CameraInit(CAMERA_PERSPECTIVE, (vec3) { 0.0f, 0.0f, 0.0f }, 90.0f);

    // Creating the basic shaders

    *GetDefaultShader(GL_VERTEX_SHADER) = CreateShader("../res/shaders/vertex.glsl", GL_VERTEX_SHADER);
    *GetDefaultShader(GL_FRAGMENT_SHADER) = CreateShader("../res/shaders/fragment.glsl", GL_FRAGMENT_SHADER);
    *GetDefaultProgram() = CreateProgram(*GetDefaultShader(GL_VERTEX_SHADER), *GetDefaultShader(GL_FRAGMENT_SHADER));

    // Render-batch

    LoadRenderBatch(1024);

    while(!WindowCloseCallback()) {  
        CameraMovement(&camera, true);   

        BeginRenderMode(&camera);
        Clear((vec4) { 0.1f, 0.1f, 0.1, 1.0f });

        RenderVoxel((vec3) { 0.0f * VOXEL_SIZE, 0.0f, 0.0f * VOXEL_SIZE }, VOXEL_SIZE, (vec4) { 1.0f, 1.0f, 1.0f, 1.0f }, true, true, false, true, true, false);
        RenderVoxel((vec3) { 0.0f * VOXEL_SIZE, 0.0f, 1.0f * VOXEL_SIZE }, VOXEL_SIZE, (vec4) { 1.0f, 1.0f, 1.0f, 1.0f }, true, true, true, false, true, false);
        RenderVoxel((vec3) { 1.0f * VOXEL_SIZE, 0.0f, 0.0f * VOXEL_SIZE }, VOXEL_SIZE, (vec4) { 1.0f, 1.0f, 1.0f, 1.0f }, true, true, false, true, false, true);
        RenderVoxel((vec3) { 1.0f * VOXEL_SIZE, 0.0f, 1.0f * VOXEL_SIZE }, VOXEL_SIZE, (vec4) { 1.0f, 1.0f, 1.0f, 1.0f }, true, true, true, false, false, true);

        EndRenderMode();
    }

    UnloadRenderBatch();
    DeleteProgram(*GetDefaultProgram());

    CloseWindow();

    return 0;
}
