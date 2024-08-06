#include "render_batch.h"

#include "SDL2/SDL.h"

#include "core.h"

extern core_data CORE;

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

    glUseProgram(CORE.shaders.shader_program_id);

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
