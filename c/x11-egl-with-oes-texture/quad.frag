#version 320 es
#extension GL_OES_EGL_image_external : require

in highp vec2 Texcoord;
out highp vec4 Color;

uniform highp samplerExternalOES yuvTexSampler;

void main(void)
{
    Color = texture2D(yuvTexSampler, Texcoord);
}
