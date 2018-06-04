/*
 * raytrace.h
 *
 * Contains functionality for ray tracing
 * */

#ifndef RAYTRACE_H
#define RAYTRACE_H

/*
 * Assumes the camera is facing in direction of (0, 0, -1).
 * Default FOV should be set to PI/2 radians
 * */
float* get_ray_origin(
		int x, int y
		float fov);

#endif
