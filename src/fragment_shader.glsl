#version 140

in vec2 pos;     // position of the pixel, both x and y in range of [-1; 1]
out vec4 color;  // the color of the pixel, RGBA

void main() {
	// TODO: draw something
	color = vec4(pos, 0, 1);
}
