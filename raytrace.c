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
	float ndc_x = (x + .5)/(float)IMG_WIDTH;
	float ndc_y = (IMG_HEIGHT-1-y + .5)/(float)IMG_HEIGHT;
	
	//now convert to screen space
	float screen_x = 2*ndc_x - 1.0f;
	float screen_y = 1.0f - 2*ndc_y;
	
	//ensure all pixels are square(?)
	float aspect_ratio = IMG_WIDTH/(float)IMG_HEIGHT;

	float fov_factor = tanf(fov/2);
	
	//create the ray we have so far
	struct Matrix *temp = new_matrix(4, 1);
	push_point(temp,
			screen_x*fov_factor*aspect_ratio,
			screen_y*fov_factor,
			-1.0f);

	struct Matrix *temp_origin = new_matrix(4, 1);
	push_point(temp_origin, 0, 0, 0);

	//now use the camera-to-world matrix
	//we will use the default position, so
	//identity matrix is used
	struct Matrix *camera_to_world = new_matrix(4, 4);
	ident(camera_to_world);
	matrix_mult(camera_to_world, temp);
	matrix_mult(camera_to_world, temp_origin);
	
	//now assemble the final ray and normalize it
	struct Ray *res = (struct Ray*)malloc(sizeof(struct Ray));
	res->origin[0] = temp_origin->m[0][0];
	res->origin[1] = temp_origin->m[1][0];
	res->origin[2] = temp_origin->m[2][0];
	
	res->direction[0] = temp->m[0][0] - res->origin[0];
	res->direction[1] = temp->m[1][0] - res->origin[1];
	res->direction[2] = temp->m[2][0] - res->origin[2];
	normalize(res->direction);
	/*
	printf("generating primary ray: %f, %f, %f\n", 
			res->direction[0],
			res->direction[1],
			res->direction[2]);
	*/
	/*
	printf("generating primary ray origin: %f, %f, %f\n", 
			res->origin[0],
			res->origin[1],
			res->origin[2]);
	*/

	res->t = -1;
	
	free_matrix(camera_to_world);
	free_matrix(temp);
	free_matrix(temp_origin);
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
	//if it is close to zero, it is parallel to the triangle
	float determinant = dot_product(p, edge1);
	//printf("determinant: %f\n", determinant);
	if (determinant < 0.0000001f) return 0;

	//res contains solutions for t, u, v
	float res[3];
	res[0] = dot_product(q, edge2)/determinant;
	res[1] = dot_product(p, s)/determinant;
	res[2] = dot_product(q, ray->direction)/determinant;
	//printf("t: %f, u: %f, v: %f\n", res[0], res[1], res[2]);

	//now check all false cases
	if (res[0] < 0) return 0;	//ray is traveling "backwards"
	if (
			res[1] < 0 ||
			res[2] < 0 ||
			res[1] > 1 ||
			//res[2] > 1 ||
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

void free_ray(struct Ray *r) {
	free(r);
}

