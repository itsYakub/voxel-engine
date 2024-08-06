#version 330 core

in vec4 vColor;
in vec2 vTexCoord;
in float vTexId;

out vec4 fFragColor;

void main() {
    fFragColor = vColor;
}