#include<math.h>
#include<stdlib.h>

#include"include/raytrace.h"
#include"include/dimen.h"
#include"include/vmath.h"

struct Ray* new_primary_ray(
		int x, int y,
		float fov) {
	//convert middle of pixel to NDC space
	//the .5 is offset for the middle
	float ndc_x = (x + .5)/IMG_WIDTH;
	float ndc_y = (y + .5)/IMG_HEIGHT;
	
	//now convert to screen space
	float screen_x = 2*ndc_x - 1;
	float screen_y = 1 - 2*ndc_y;
	
	//ensure all pixels are square(?)
	float aspect_ratio = IMG_WIDTH/IMG_HEIGHT;

	float fov_factor = tanf(fov/2);
	
	struct Ray *res = (struct Ray*)malloc(sizeof(struct Ray));
	res->direction[0] = screen_x*fov_factor*aspect_ratio;
	res->direction[1] = screen_y*fov_factor;
	res->direction[2] = -1;
	
	res->origin[0] = 0.0f;
	res->origin[1] = 0.0f;
	res->origin[2] = 0.0f;

	res->t = 0;
	
	return res;
}

char ray_triangle_intersect(
		struct Ray *ray,
		float *t,
		float x1, float y1, float z1,
		float x2, float y2, float z2,
		float x3, float y3, float z3) {
	//triangle vertices
	float a[] = {x1, y1, z1};
	float b[] = {x2, y2, z2};
	float c[] = {x3, y3, z3};
	
	float s[3], edge1[3], edge2[3];
	subtract_vectors(s, ray->origin, a);
	subtract_vectors(edge1, b, a);
	subtract_vectors(edge2, c, a);
	
	//some repeated calculations
	float p[3], q[3];
	cross_product(p, ray->direction, edge2);
	cross_product(q, s, edge1);

	//if triple product of direction, edge2, and edge1
	//is negative, the ray hits the triangle from behind.
	//this is the backface, so don't include it
	float determinant = dot_product(p, edge1);
	if (determinant < 0) return 0;

	//res contains solutions for t, u, v
	float res[3];
	res[0] = dot_product(q, edge2)/determinant;
	res[1] = dot_product(p, s)/determinant;
	res[2] = dot_product(q, ray->direction)/determinant;

	//now check all false cases
	if (res[0] < 0) return 0;	//ray is traveling "backwards"
	if (
			res[1] < 0 ||
			res[2] < 0 ||
			res[1] > 1 ||
			res[2] > 1 ||
			res[1] + res[2] > 1) {
		//point is not inside triangle
		return 0;
	}

	//we'll want to save this
	//we don't place it directly in ray because
	//the ray could hit multiple triangles,
	//and only a certain (ie smallest)
	//value of t would be wanted
	*t = res[0];
	
	return 1;
}


