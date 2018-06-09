/*
 * raytrace.h
 *
 * Contains functionality for ray tracing
 * */

#ifndef RAYTRACE_H
#define RAYTRACE_H

#include"draw.h"
#include"shapes.h"

struct Ray {
	float origin[3];
	float direction[3];
	float t;
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

//input origin and direction
struct Ray *new_ray(
		float orig_x, float orig_y, float orig_z,
		float d_x, float d_y, float d_z);

//renders scene using ray tracing
void render(Frame f, struct Object **objs,
		struct Light **lights,
		int obj_count, int light_count);

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

//returns the color at a pixel
struct Pixel* cast_ray(int x, int y,
		struct Object **objs,
		struct Light **lights,
		int obj_count, int light_count);

struct Ray* new_reflection_ray(struct Ray *init);

struct Ray* new_refraction_ray(struct Ray *init);

//tests if the point hit by a ray is in the shadow of an object
char in_shadow(struct Ray *init, float bias,
		struct Object **objs,
		struct Light *light,
		int obj_count, int light_count);

void free_ray(struct Ray *r);

#endif
