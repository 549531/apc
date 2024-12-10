#version 140

uniform uint time;  // time since program start in milliseconds
in vec3 pos;        // position of the pixel, both x and y in range of [-1; 1]
out vec4 color;     // the color of the pixel, RGBA

vec4 quat(vec3 axis, float angle) {
	return vec4(normalize(axis) * sin(angle / 2), cos(angle / 2));
}

vec3 rotate(vec3 p, vec4 q) {
	vec3 t = cross(2 * q.xyz, p);
	return p + q.w * t + cross(q.xyz, t);
}

float cube(vec3 pos, vec3 center, vec3 size) {
	vec3 dist = abs(pos - center) - size;
	return max(dist.x, max(dist.y, dist.z));
}

float sdf(vec3 p) {
	vec3 c = vec3(.5, 0, -.5);
	return min(  //
	    cube(rotate(p - c, quat(vec3(1), float(time) / 1000)) + c, c,
		 vec3(.25)),
	    cube(p, vec3(-.5, 0, -5), vec3(.25)));
}

const float minRayDist = .001;
const float maxRayDist = 100;

vec3 normal(vec3 p) {
	vec2 step = vec2(minRayDist, 0);
	return normalize(vec3(  //
	    sdf(p + step.xyy) - sdf(p - step.xyy),
	    sdf(p + step.yxy) - sdf(p - step.yxy),
	    sdf(p + step.yyx) - sdf(p - step.yyx)));
}

const vec3 camera = vec3(0, 0, 5);

void main() {
	vec3 dir = normalize(pos - camera);

	float dist = 0;
	for (;;) {
		vec3 p = pos + dir * dist;
		float step = sdf(p);
		dist += step;
		if (dist > maxRayDist) {
			color.rgb = vec3(.5);
		} else if (step <= minRayDist) {
			color.rgb = abs(normal(p));
		} else {
			continue;
		}
		break;
	}

	color.a = 1;
}
