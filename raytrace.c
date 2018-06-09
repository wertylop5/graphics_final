#include<math.h>
#include<stdlib.h>

#include<float.h>

#include"include/raytrace.h"
#include"include/dimen.h"
#include"include/vmath.h"
#include"include/output.h"

struct Ray* new_primary_ray(
		int x, int y,
		float fov) {
	//convert middle of pixel to NDC space
	//the .5 is offset for the middle
	float ndc_x = (x + .5)/(float)IMG_WIDTH;
	float ndc_y = (IMG_HEIGHT-1-y + .5)/(float)IMG_HEIGHT;

	//now convert to screen space
	float screen_x = (2*ndc_x - 1.0f);
	float screen_y = (1.0f - 2*ndc_y);

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
	//struct Matrix *move_m = move(250, 250, -20);
	//matrix_mult(move_m, camera_to_world);
	//free_matrix(move_m);

	matrix_mult(camera_to_world, temp);
	matrix_mult(camera_to_world, temp_origin);

	//now assemble the final ray and normalize it
	struct Ray *res = (struct Ray*)malloc(sizeof(struct Ray));
	res->origin[0] = temp_origin->m[0][0];
	res->origin[1] = temp_origin->m[1][0];
	res->origin[2] = temp_origin->m[2][0];

	res->direction[0] = temp->m[0][0];
	res->direction[1] = temp->m[1][0];
	res->direction[2] = temp->m[2][0];
	normalize(res->direction);
	/*
	   printf("generating primary ray: %f, %f, %f\n", 
	   res->direction[0],
	   res->direction[1],
	   res->direction[2]);


	   printf("generating primary ray origin: %f, %f, %f\n", 
	   res->origin[0],
	   res->origin[1],
	   res->origin[2]);
	   */

	res->t = FLT_MAX;

	free_matrix(camera_to_world);
	free_matrix(temp);
	free_matrix(temp_origin);
	return res;
}

void render(Frame f, struct Object **objs,
		struct Light **lights,
		int obj_count, int light_count) {
	clear_frame(f, 1);
	//printf("poly count: %d\n", );
	printf("lights: %d, objs: %d\n", light_count, obj_count);

	struct Pixel pixel;
	pixel_color(&pixel, 0, 255, 0);
	int w, h, cur_poly, cur_obj;
	for (h = 0; h < IMG_HEIGHT; h++) {//height loop
		for (w = 0; w < IMG_WIDTH; w++) {//width loop
			struct Ray *prim = new_primary_ray(w, h, M_PI/4);
			float t;
			int closest_poly = -1;

			for (cur_obj = 0; cur_obj < obj_count; cur_obj++) {//obj loop
				struct Matrix *polys = objs[cur_obj]->polys;
				for (cur_poly = 0; cur_poly < polys->back; cur_poly+=3) {//poly loop
					t = FLT_MAX;

					/*
					   printf("iter %f, %f, %f\n%f, %f, %f\n%f, %f, %f\n\n",
					   polys->m[0][cur_poly],
					   polys->m[1][cur_poly],
					   polys->m[2][cur_poly],
					   polys->m[0][cur_poly+1],
					   polys->m[1][cur_poly+1],
					   polys->m[2][cur_poly+1],
					   polys->m[0][cur_poly+2],
					   polys->m[1][cur_poly+2],
					   polys->m[2][cur_poly+2]
					   );

*/
					if (ray_triangle_intersect(
							prim,
							&t,
							polys->m[0][cur_poly],
							polys->m[1][cur_poly],
							polys->m[2][cur_poly],
							polys->m[0][cur_poly+1],
							polys->m[1][cur_poly+1],
							polys->m[2][cur_poly+1],
							polys->m[0][cur_poly+2],
							polys->m[1][cur_poly+2],
							polys->m[2][cur_poly+2])) {

						if (t < prim->t) {
							prim->t = t;
							closest_poly = cur_poly;
						}

					}

				}//end poly loop


				//display only the closest polygon
				if (closest_poly > 0) {
					//printf("final t: %f\n", prim->t);
					float normal[3];
					find_norm(polys, closest_poly, closest_poly+1,
							closest_poly+2, normal);

					//light loop
					int cur_light;
					for (cur_light = 0; cur_light < light_count; cur_light++) {
						struct Pixel *color = get_lighting_matte(lights[cur_light], normal, .5, .5);
						//printf("color: %d, %d, %d\n", color->r, color->g, color->b);
						plot_point_trace(f, w, h, color);
						free(color);
					}//end light loop
				}

			}//end obj loop

			free_ray(prim);
		}//end width loop
	}//end height loop

	//draw_polygons(f, z, polys, &pixel, l, view_vect);
	display(f);
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
	//printf("got %f, %f, %f\n", z1, z2, z3);

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
	/*
	   printf("%f, %f, %f\n\
	   %f, %f, %f\n\
	   %f, %f, %f\n\n",
	   x1, y1, z1,
	   x2, y2, z2,
	   x3, y3, z3);
	   */
	return 1;
}

void free_ray(struct Ray *r) {
	free(r);
}

