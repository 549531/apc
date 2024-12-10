#version 140

uniform uint time;  // time since program start in milliseconds
in vec3 pos;        // position of the pixel, both x and y in range of [-1; 1]
out vec4 color;     // the color of the pixel, RGBA

// create a quaternion from a rotation @axis & @angle
vec4 quat(vec3 axis, float angle) {
	return vec4(normalize(axis) * sin(angle / 2), cos(angle / 2));
}

// rotate @point using @quaternion
vec3 rotate(vec3 point, vec4 quaternion) {
	vec3 t = cross(2 * quaternion.xyz, point);
	return point + quaternion.w * t + cross(quaternion.xyz, t);
}

// objects that will be drawn
const uint SKY = 0u;
const uint FIRST_CUBE = 1u;
const uint SECOND_CUBE = 2u;

struct sdf {
	uint object;
	float dist;
};

sdf sdf_union(sdf a, sdf b) {
	return a.dist < b.dist ? a : b;
}

// calculate SDF of a cube with given @size in point @pos
sdf cube(uint id, vec3 size, vec3 pos) {
	vec3 dist = abs(pos) - size;
	return sdf(id, max(dist.x, max(dist.y, dist.z)));
}

// the scene to draw
sdf scene(vec3 p) {
	return sdf_union(  //
	    cube(FIRST_CUBE, vec3(.25),
		 rotate(p - vec3(.5, 0, -.5),  //
			quat(vec3(1), float(time) / 1000))),
	    cube(SECOND_CUBE, vec3(.25),
		 rotate(p - vec3(-.5, 0, -5),  //
			quat(vec3(1, -1, -1), radians(30)))));
}

const float minRayDist = .001;
const float maxRayDist = 100;

vec3 normal(vec3 p) {
	vec2 step = vec2(minRayDist, 0);
	return normalize(vec3(  //
	    scene(p + step.xyy).dist - scene(p - step.xyy).dist,
	    scene(p + step.yxy).dist - scene(p - step.yxy).dist,
	    scene(p + step.yyx).dist - scene(p - step.yyx).dist));
}

const vec3 camera = vec3(0, 0, 5);
const vec3 sun = vec3(-5, 5, 5);

const vec3 GRAY = vec3(.5);
const vec3 GREEN = vec3(.0859375, .859375, .53125);
const vec3 RED = vec3(1, 0, 0);

vec3 color_of(uint object, vec3 p) {
	float brightness = max(0, dot(normal(p), normalize(sun - p)));
	switch (object) {
	case SKY: return GRAY;
	case FIRST_CUBE: return abs(normal(p));
	case SECOND_CUBE: return GREEN * brightness;
	}
}

void main() {
	vec3 dir = normalize(pos - camera);

	float dist = 0;
	for (;;) {
		vec3 p = pos + dir * dist;
		sdf sdf = scene(p);
		dist += sdf.dist;
		if (dist > maxRayDist) {
			color.rgb = color_of(SKY, p);
		} else if (sdf.dist <= minRayDist) {
			color.rgb = color_of(sdf.object, p);
		} else {
			continue;
		}
		break;
	}

	color.a = 1;
}
