#if !defined (RENDER_BATCH_H)
#define RENDER_BATCH_H

#include "glad/glad.h"
#include "cglm/types.h"

typedef struct {
    vec3 position;
    vec4 color;
    vec2 texcoord;
    GLint texid;
} vert;

void LoadRenderBatch(const GLuint triangles_count);
void UnloadRenderBatch();
void DrawRenderBatch();
void PushRenderBatchVertexData(vec3* positions, vec4* colors, vec2* texcoord, GLint texid, GLuint data_count);
void PushRenderBatchIndexData(GLuint* index_data, GLuint data_count);

#endif // RENDER_BATCH_H