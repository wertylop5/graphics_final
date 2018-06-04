#include"include/raytrace.h"
#include"include/dimen.h"

struct Ray* new_primary_ray(
		int x, int y
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
	
	struct Ray *res = (struct Ray*)malloc(3*sizeof(struct Ray));
	res->direction[0] = screen_x*fov_factor*aspect_ratio;
	res->direction[1] = screen_y*fov_factor;
	res->direction[2] = -1;
	
	res->origin[0] = 0f;
	res->origin[1] = 0f;
	res->origin[2] = 0f;
	
	return res;
}

char ray_triangle_intersect(
		struct Ray *ray,
		float x1, float y1, float z1,
		float x2, float y2, float z2,
		float x3, float y3, float z3) {
	
}


