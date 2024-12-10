#ifndef SHADERS_H
#define SHADERS_H

const char* vShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 in_pos;
out vec2 frag_pos;

void main()
{
    frag_pos = in_pos;
    gl_Position = vec4(in_pos, 0.0, 1.0);
}
)";

const char* fShaderSource = R"(
#version 330 core
in vec2 frag_pos;
out vec4 out_color;

uniform float time;

void main()
{
    vec2 uv = frag_pos * 0.5 + 0.5;
    float r = 0.5 + 0.5 * sin(uv.x * 10.0 + time);
    float g = 0.5 + 0.5 * sin(uv.y * 10.0 + time * 1.5);
    float b = 0.5 + 0.5 * sin((uv.x + uv.y) * 10.0 - time);
    out_color = vec4(r, g, b, 1.0);
}
)";

#endif // SHADERS_H
