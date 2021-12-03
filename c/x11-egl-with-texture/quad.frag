#version 320 es

in highp vec2 Texcoord;
out highp vec4 Color;

uniform highp sampler2D tex;

void main(void)
{
    Color = texture(tex, Texcoord);
}
