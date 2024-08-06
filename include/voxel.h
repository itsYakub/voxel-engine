#if !defined(VOXEL_H)
#define VOXEL_H

#include <stdbool.h>

#include "cglm/types.h"

void RenderVoxel(vec3 position, int size, vec4 tint, bool draw_top, bool draw_down, bool draw_front, bool draw_back, bool draw_left, bool draw_right);

#endif // VOXEL_H