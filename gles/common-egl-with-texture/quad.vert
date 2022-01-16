#version 320 es

layout (location = 0) in highp vec4 position;
layout (location = 1) in highp vec2 texcoord;

out highp vec2 Texcoord;

uniform mat4 modelMat;

void main(void) {
    Texcoord = texcoord;
    gl_Position = modelMat * vec4(position.xyz * 0.8f, 1.0f);
}
