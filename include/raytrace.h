/*
 * raytrace.h
 *
 * Contains functionality for ray tracing
 * */

#ifndef RAYTRACE_H
#define RAYTRACE_H

struct Ray {
	float origin[3];
	float direction[3];
	int t;
};

/*
 * Assumes the camera is facing in direction of (0, 0, -1).
 * Default FOV should be set to PI/2 radians
 *
 * Returns a ray going through the center of the pixel at
 * x, y
 * */
struct Ray* new_primary_ray(
		int x, int y,
		float fov);

/*
Uses the Moller-Trumbore algorithm

x, y, z should be the coordinates of a vertex of the desired triangle
returns 0 if no intersection, 1 if there is
*/
char ray_triangle_intersect(
		struct Ray *ray,
		float *t,
		float x1, float y1, float z1,
		float x2, float y2, float z2,
		float x3, float y3, float z3);

#endif
