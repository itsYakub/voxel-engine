#include "voxel.h"

#include "glad/glad.h"

#include "render_batch.h"

void RenderVoxel(vec3 position, int size, vec4 tint, bool draw_top, bool draw_down, bool draw_front, bool draw_back, bool draw_left, bool draw_right) {
    // Layout of a basic voxel:
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

    if(draw_front) { // Face: FRONT
        vec3 face_vertices[4] = {
            { vertex_positions[2][0], vertex_positions[2][1], vertex_positions[2][2] },
            { vertex_positions[3][0], vertex_positions[3][1], vertex_positions[3][2] },
            { vertex_positions[6][0], vertex_positions[6][1], vertex_positions[6][2] },
            { vertex_positions[7][0], vertex_positions[7][1], vertex_positions[7][2] },
        };

        GLfloat factor = 0.90f;

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

        GLfloat factor = 0.90f;

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

        GLfloat factor = 0.85f;

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

        GLfloat factor = 0.85f;

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