#version 140

in vec2 in_pos;  // set by main.cpp
out vec2 pos;    // used in fragment shader

void main() {
	pos = in_pos;
	gl_Position = vec4(in_pos, 0, 1);
}
