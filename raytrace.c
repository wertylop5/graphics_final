#include"include/raytrace.h"
#include"include/dimen.h"

float* get_ray_origin(
		int x, int y
		float fov) {
	float ndc_x = (x + .5)/IMG_WIDTH;
	float ndc_y = (y + .5)/IMG_HEIGHT;
	
	float screen_x = 2*ndc_x - 1;
	float screen_y = 1 - 2*ndc_y;
	
	float aspect_ratio = IMG_WIDTH/IMG_HEIGHT;
	
	float *res = (float *)malloc(3*sizeof(float));
	res[0] = ;
	res[1] = ;
	res[2] = -1;
}

