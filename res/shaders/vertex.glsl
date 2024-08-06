#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in float aTexId;

out vec4 vColor;
out vec2 vTexCoord;
out float vTexId;

uniform mat4 uMatrixProjection;
uniform mat4 uMatrixView;

void main() {
    gl_Position = uMatrixProjection * uMatrixView * vec4(aPosition, 1.0f);

    vColor = aColor;
    vTexCoord = aTexCoord;
    vTexId = aTexId;
}