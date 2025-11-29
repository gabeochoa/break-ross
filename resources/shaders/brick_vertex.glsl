#version 330

in vec2 vertexPosition;

uniform vec2 brickStart;
uniform float brickCellSize;
uniform vec2 worldSize;
uniform ivec2 gridPos;

out vec2 fragUV;

void main() {
    vec2 worldPos = brickStart + vec2(gridPos) * brickCellSize + vertexPosition * brickCellSize;
    vec2 screenPos = (worldPos / worldSize) * 2.0 - 1.0;
    screenPos.y = -screenPos.y;
    
    gl_Position = vec4(screenPos, 0.0, 1.0);
    fragUV = vec2(gridPos);
}
