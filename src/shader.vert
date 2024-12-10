#version 140

in vec3 in_pos;  // set by main.cpp
out vec3 pos;    // used in fragment shader

void main() {
	pos = in_pos;
	gl_Position = vec4(in_pos, 1);
}
