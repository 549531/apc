#version 140

uniform uint time;  // time since program start in milliseconds
in vec3 pos;        // position of the pixel, both x and y in range of [-1; 1]
out vec4 color;     // the color of the pixel, RGBA

// create a quaternion from a rotation @axis & an @angle
vec4 quat(vec3 axis, float angle) {
	return vec4(normalize(axis) * sin(angle / 2), cos(angle / 2));
}

vec4 quatmul(vec4 q1, vec4 q2) {
	return vec4(q1.w * q2.xyz + q2.w * q1.xyz + cross(q1.xyz, q2.xyz),
		    q1.w * q2.w - dot(q1.xyz, q2.xyz));
}

// rotate the plane using @quaternion
vec3 rotate(vec4 quaternion, vec3 point) {
	vec3 t = cross(2 * quaternion.xyz, point);
	return point + quaternion.w * t + cross(quaternion.xyz, t);
}

// move 2D plane by @d
vec2 translate2(vec2 d, vec2 point) {
	return point - d;
}

// move 3D plane by @d
vec3 translate3(vec3 d, vec3 point) {
	return point - d;
}

// objects that will be drawn
const uint SKY = 0u;
const uint TIRE = 1u;
const uint METAL = 2u;

struct sdf {
	uint object;
	float dist;
};

sdf sdf_union(sdf a, sdf b) {
	return a.dist < b.dist ? a : b;
}

// create a cube with given @size at point @pos
float cube(vec3 size, vec3 pos) {
	vec3 dist = abs(pos) - size;
	return max(max(dist.x, dist.y), dist.z);
}

// create a 2d rectangle with dimensions @size at point @pos
float rectangle(vec2 size, vec2 pos) {
	vec2 d = abs(pos) - size;
	return max(d.x, d.y);
}

// create a 2d circle with radius @r at point @pos
float circle(float r, vec2 pos) {
	return length(pos) - r;
}

float ellipse(vec2 r, vec2 pos) {
	return circle(r.x * r.y, r * pos);
}

// create a 3d object by revolution of a 2d blueprint around the Y axis
// e.g. to create a torus: circle(.5, translate(vec2(1, 0), revolved(pos)))
vec2 revolved(vec3 pos) {
	return vec2(length(pos.xz), pos.y);
}

float cylinder(float r, float h, vec3 pos) {
	float d = circle(r, pos.xy);
	vec2 w = vec2(d, abs(pos.z) - h);
	return min(max(w.x, w.y), 0) + length(max(w, 0));
}

float tire(vec3 pos) {
	const float r = 1;
	const float w = .125;
	const float h = .0625;
	const vec2 axis = vec2(1, 0);
	vec2 p = revolved(pos);
	float base = rectangle(vec2(h, w), translate2(vec2(r, 0), p));
	float tire = ellipse(vec2(w, w / 2), translate2(vec2(r + h, 0), p));
	float d = min(base, tire);
	for (int i = 0; i < 360; i += 40) {
		d = min(d,
			cylinder(w / 5, r,
				 translate3(
				     vec3(.125, 0, 0),
				     rotate(quatmul(quat(axis.xyy, radians(5)),
						    quat(axis.yxy, radians(i))),
					    pos))));
	}
	return d;
}

// the scene to draw
sdf scene(vec3 p) {
	sdf s;
	s = sdf(TIRE, tire(rotate(quat(vec3(1), time / 500.),
				  translate3(vec3(0, 0, -10), p))));
	return s;
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
	case TIRE: return GREEN * brightness;
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
