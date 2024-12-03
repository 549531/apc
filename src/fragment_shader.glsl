#version 140

in vec2 pos;     // position of the pixel, both x and y in range of [-1; 1]
out vec4 color;  // the color of the pixel, RGBA

float rectangle(vec2 pos, vec2 center, vec2 size) {
	vec2 dist = abs(pos - center) - size;
	return max(dist.x, dist.y);
}

void main() {
	float sdf = rectangle(pos, vec2(0), vec2(.25));
	color = vec4(vec3(sdf <= 0), 1);
}
