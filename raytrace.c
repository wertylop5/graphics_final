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
	push_point(temp_origin,
		options.camera_origin[0],
		options.camera_origin[1],
		options.camera_origin[2]);

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

	res->t = FLT_MAX;

	free_matrix(camera_to_world);
	free_matrix(temp);
	free_matrix(temp_origin);
	return res;
}

struct Ray* new_reflection_ray(
		struct Ray *init,
		float *norm) {
	struct Ray *res = (struct Ray *)malloc(sizeof(struct Ray));
	float dot = dot_product(init->direction, norm);
	
	//the reflected ray starts where the original ray hit
	//the object
	if (dot < 0) {
		res->origin[0] =
			init->direction[0]*init->t +
			init->origin[0] + norm[0]*options.bias;
		res->origin[1] =
			init->direction[1]*init->t +
			init->origin[1] + norm[1]*options.bias;
		res->origin[2] =
			init->direction[2]*init->t +
			init->origin[2] + norm[2]*options.bias;
	}
	else {
		//total internal reflection, so displace it inwards
		res->origin[0] =
			init->direction[0]*init->t +
			init->origin[0] - norm[0]*options.bias;
		res->origin[1] =
			init->direction[1]*init->t +
			init->origin[1] - norm[1]*options.bias;
		res->origin[2] =
			init->direction[2]*init->t +
			init->origin[2] - norm[2]*options.bias;
		
		norm[0] *= -1;
		norm[1] *= -1;
		norm[2] *= -1;
	}
	

	//compute reflected ray direction
	res->direction[0] =
		init->direction[0] - 
		2*dot*norm[0];
	res->direction[1] = 
		init->direction[1] - 
		2*dot*norm[1];
	res->direction[2] = 
		init->direction[2] - 
		2*dot*norm[2];
	normalize(res->direction);

	res->t = FLT_MAX;
	
	return res;
}

struct Ray* new_refraction_ray(
		struct Ray *init,
		float *norm,
		float ior) {
	struct Ray *res = (struct Ray *)malloc(sizeof(struct Ray));
	
	float c1 = dot_product(init->direction, norm);
	clamp(-1, 1, &c1);

	
	float eta;

	//is ray going into material or out of it
	if (c1 > 0) {	//out of material
		res->origin[0] =
			init->direction[0]*init->t +
			init->origin[0] + norm[0]*options.bias;
		res->origin[1] =
			init->direction[1]*init->t +
			init->origin[1] + norm[1]*options.bias;
		res->origin[2] =
			init->direction[2]*init->t +
			init->origin[2] + norm[2]*options.bias;
		
		norm[0] *= -1;
		norm[1] *= -1;
		norm[2] *= -1;

		eta = ior/1;
	}
	else {	//into material
		res->origin[0] =
			init->direction[0]*init->t +
			init->origin[0] - norm[0]*options.bias;
		res->origin[1] =
			init->direction[1]*init->t +
			init->origin[1] - norm[1]*options.bias;
		res->origin[2] =
			init->direction[2]*init->t +
			init->origin[2] - norm[2]*options.bias;
		
		eta = 1/ior;
	}
	float inner = (1 - eta*eta) * (1 - c1*c1);
	
	//total internal reflection, no refraction
	if (inner < 0) {
		free_ray(res);
		res = 0;
		return 0;
	}
	
	float c2 = sqrtf(inner);
	
	//no dealing with angles, wow!
	res->direction[0] =
		c1*init->direction[0] +
		norm[0]*(eta*c1 - c2);
	res->direction[1] =
		c1*init->direction[1] +
		norm[1]*(eta*c1 - c2);
	res->direction[2] =
		c1*init->direction[2] +
		norm[2]*(eta*c1 - c2);
	
	return res;
}

float reflection_ray_strength(
		struct Ray *init,
		float *norm,
		float ior) {
	float cosi = dot_product(norm, init->direction);
	clamp(-1, 1, &cosi);

	float eta, n1, n2;

	//is ray going into material or out of it
	if (cosi > 0) {	//out of material
		n1 = ior;
		n2 = 1;
	}
	else {	//into material
		n1 = 1;
		n2 = ior;
	}
	eta = n1/n2;
	
	float sint = eta*eta*fmaxf(1 - cosi*cosi, 0);
	
	//total internal reflection
	if (sint >= 1) return 1;
	float cost = sqrtf(fmaxf(1 - sint, 0));
	
	cosi = fabsf(cosi);
	
	float perpendicular = powf(
		(n1*cosi - n2*cost) / (n1*cosi + n2*cost), 2);
	float parallel = powf(
		(n1*cost - n2*cosi) / (n1*cost - n2*cosi), 2);
	
	return .5 * (parallel + perpendicular);
}

void render(Frame f, struct Object **objs,
		struct Light **lights,
		int obj_count, int light_count) {
	struct Pixel bkgd_pixel;
	pixel_color(&bkgd_pixel,
		options.bkgd_color[RED],
		options.bkgd_color[GREEN],
		options.bkgd_color[BLUE]);
	init_frame(f, &bkgd_pixel);

	int temp, polycount = 0;
	for (temp = 0; temp < obj_count; temp++) {
		polycount += objs[temp]->polys->back;
	}
	
	printf("lights: %d, objs: %d, polycount: %d, depth: %d\n",
		light_count, obj_count,
		polycount, options.recursion_depth);

	struct Pixel *pixel;
	int w, h;
	for (h = 0; h < IMG_HEIGHT; h++) {//height loop
		for (w = 0; w < IMG_WIDTH; w++) {//width loop
			pixel = cast_ray(w, h, objs, lights,
				obj_count, light_count,
				options.recursion_depth);
			
			if (pixel != 0) {
				plot_point_trace(f, w, h, pixel);
				free(pixel);
			}
			/*
			else {
				plot_point_trace(f, w,
					h, &bkgd_pixel);
			}
			*/
		}//end width loop
	}//end height loop
}

struct Pixel* cast_ray(int x, int y,
		struct Object **objs,
		struct Light **lights,
		int obj_count, int light_count,
		int depth_count) {
	struct Ray *prim = new_primary_ray(x, y, options.fov);
	
	return trace(prim, objs, lights, obj_count, light_count,
			depth_count);
}

struct Pixel *trace(struct Ray *ray,
		struct Object **objs,
		struct Light **lights,
		int obj_count, int light_count,
		int depth_count) {
	if (depth_count <= 0) {
		return 0;
	}
	
	struct Pixel *color = 0;
	int cur_poly, cur_obj;
	float t, u, v;
	float u_closest, v_closest;
	int closest_poly = -1, closest_obj = -1;

	for (cur_obj = 0; cur_obj < obj_count; cur_obj++) {//obj loop
		struct Matrix *polys = objs[cur_obj]->polys;
		for (		cur_poly = 0;
				cur_poly < polys->back;
				cur_poly+=3) {//poly loop
			t = FLT_MAX;
			
			if (ray_triangle_intersect(
					ray,
					&t, &u, &v,
					polys->m[0][cur_poly],
					polys->m[1][cur_poly],
					polys->m[2][cur_poly],
					polys->m[0][cur_poly+1],
					polys->m[1][cur_poly+1],
					polys->m[2][cur_poly+1],
					polys->m[0][cur_poly+2],
					polys->m[1][cur_poly+2],
					polys->m[2][cur_poly+2])) {
				
				if (t < ray->t) {
					ray->t = t;
					closest_poly = cur_poly;
					closest_obj = cur_obj;
					u_closest = u;
					v_closest = v;
				}

			}

		}//end poly loop

		//display only the closest polygon
		if (closest_poly > -1) {
		color = (struct Pixel *)malloc(sizeof(struct Pixel));
		pixel_color(color, 0, 0, 0);
		//printf("final t: %f\n", prim->t);
		//float normal[3];
		//find_norm(objs[closest_obj]->polys,
			//closest_poly, closest_poly+1,
			//closest_poly+2, normal);

		float vertex1[] = {
			objs[closest_obj]->polys->
			m[0][closest_poly],
			objs[closest_obj]->polys->
			m[1][closest_poly],
			objs[closest_obj]->polys->
			m[2][closest_poly]};
		float vertex2[] = {
			objs[closest_obj]->polys->
			m[0][closest_poly+1],
			objs[closest_obj]->polys->
			m[1][closest_poly+1],
			objs[closest_obj]->polys->
			m[2][closest_poly+1]};
		float vertex3[] = {
			objs[closest_obj]->polys->
			m[0][closest_poly+2],
			objs[closest_obj]->polys->
			m[1][closest_poly+2],
			objs[closest_obj]->polys->
			m[2][closest_poly+2]};

		char *vertex_key1 =
			(char *)malloc(256);
		snprintf(vertex_key1, 256,
			"%.3f,%.3f,%.3f", 
			vertex1[0],
			vertex1[1],
			vertex1[2]);
		
		char *vertex_key2 =
			(char *)malloc(256);
		snprintf(vertex_key2, 256,
			"%.3f,%.3f,%.3f", 
			vertex2[0],
			vertex2[1],
			vertex2[2]);
		
		char *vertex_key3 =
			(char *)malloc(256);
		snprintf(vertex_key3, 256,
			"%.3f,%.3f,%.3f", 
			vertex3[0],
			vertex3[1],
			vertex3[2]);
		
		struct Vertex *v1 =
			find_vertex(
				&objs[closest_obj]->vertex_table,
				vertex_key1);
		struct Vertex *v2 =
			find_vertex(
				&objs[closest_obj]->vertex_table,
				vertex_key2);
		struct Vertex *v3 =
			find_vertex(
				&objs[closest_obj]->vertex_table,
				vertex_key3);
		/*
		printf("%f,%f,%f\n",
			norm->normal[0],
			norm->normal[1],
			norm->normal[2]);
			*/
		float normal[] = {
			v1->normal[0]*(1-u_closest-v_closest) +
			v2->normal[0]*(u_closest) +
			v3->normal[0]*(v_closest),
			v1->normal[1]*(1-u_closest-v_closest) +
			v2->normal[1]*(u_closest) +
			v3->normal[1]*(v_closest),
			v1->normal[2]*(1-u_closest-v_closest) +
			v2->normal[2]*(u_closest) +
			v3->normal[2]*(v_closest)
		};

		int cur_light;
		for (	cur_light = 0;
			cur_light < light_count;
			cur_light++) {//light loop
			//if in shadow, use only ambient light
			switch(objs[closest_obj]->behavior) {
			case DIFFUSE_AND_GLOSSY:
			{
				if (in_shadow(ray, options.bias,
						objs,
						lights[cur_light],
						obj_count)) {
					
					//only use ambient light
					struct Pixel *temp_color =
						calc_ambient(
						lights[cur_light],
						.3);
					add_pixel(color, temp_color);
					free(temp_color);
					free_ray(ray);
					return color;
				}
				
				struct Pixel *temp_color =
					get_lighting_matte(
					lights[cur_light],
					//normal, .3, .5);
					normal, .3, .5);
				
				add_pixel(color, temp_color);
				
				free(temp_color);
				temp_color = 0;
			}
			break;
			case REFLECTION_AND_REFRACTION:
			{
				struct Pixel *refract_color = 0;
				
				//just use glass for now
				struct Ray *refract =
					new_refraction_ray(
					ray, normal, 1.5);
				if (refract != 0) {
				refract_color =
					trace(refract,
						objs, lights,
						obj_count,
						light_count,
						depth_count-1);
				}
				
				struct Ray *reflect =
					new_reflection_ray(
					ray, normal);
				struct Pixel *reflect_color = 
					trace(reflect,
						objs, lights,
						obj_count,
						light_count,
						depth_count-1);
				
				//if nothing hit
				if (refract_color == 0 &&
						reflect_color == 0) {
					free(color);
					color = 0;
				}
				
				float reflect_strength = 
					reflection_ray_strength(
					ray, normal, 1.5);
				
				if (refract_color != 0) {
					float refract_strength =
						1 - reflect_strength;
					
					refract_color->r *= refract_strength;
					refract_color->g *= refract_strength;
					refract_color->b *= refract_strength;
					
					add_pixel(color,
						refract_color);
					free(refract_color);
					refract_color = 0;
				}
				if (reflect_color != 0) {
					reflect_color->r *= reflect_strength;
					reflect_color->g *= reflect_strength;
					reflect_color->b *= reflect_strength;
					
					add_pixel(color,
						reflect_color);
					free(reflect_color);
					reflect_color = 0;
				}
			}
			break;
			case REFLECTION:
			{
				struct Ray *reflect =
					new_reflection_ray(
					ray, normal);
				
				struct Pixel *temp_color = 
					trace(reflect,
						objs, lights,
						obj_count,
						light_count,
						depth_count-1);
				
				//if the reflected ray(s)
				//hit something
				if (temp_color != 0) {
					add_pixel(color, temp_color);
					free(temp_color);
					temp_color = 0;
				}
				else{
				//use background color, aka
				//return null
				if (color != 0) {
					free(color);
					color = 0;
				}
				}
			}
			break;
			case PLANE:
			{
				if (in_shadow(ray, options.bias,
						objs,
						lights[cur_light],
						obj_count)) {
					//use a special "dark" color
					pixel_color(color,
						60, 60, 60);
					free_ray(ray);
					return color;
				}
				
				//display planes in gray
				pixel_color(color,
					127, 127, 127);
				
				struct Ray *reflect =
					new_reflection_ray(
					ray, normal);
				
				struct Pixel *temp_color = 
					trace(reflect,
						objs, lights,
						obj_count,
						light_count,
						depth_count-1);
				
				//if the reflected ray(s)
				//hit something
				if (temp_color != 0) {
					add_pixel(color, temp_color);
					free(temp_color);
					temp_color = 0;
				}
			}
			break;
			}//end obj behavior switch
		}//end light loop
		}//end closest poly if

	}//end obj loop

	free_ray(ray);
	return color;
}

char ray_triangle_intersect(
		struct Ray *ray,
		float *t, float *u, float *v,
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
	if (u != 0) *u = res[1];
	if (v != 0) *v = res[2];
	
	return 1;
}

char in_shadow(struct Ray *init, float bias,
		struct Object **objs,
		struct Light *light,
		int obj_count) {
	
	struct Ray shadow;
	float t;
	int cur_obj, cur_poly;
	for (cur_obj = 0; cur_obj < obj_count; cur_obj++) {//obj loop
	for (	cur_poly = 0;
			cur_poly < objs[cur_obj]->polys->back;
			cur_poly+=3) {//poly loop
		float norm[3];
		find_norm(objs[cur_obj]->polys,
				cur_poly, cur_poly+1, cur_poly+2,
				norm);
		
		//bias is used to avoid self-intersection due to
		//rounding errors
		shadow.origin[0] =
				init->direction[0]*init->t +
				init->origin[0] + norm[0]*bias;
		shadow.origin[1] =
				init->direction[1]*init->t +
				init->origin[1] + norm[1]*bias;
		shadow.origin[2] =
				init->direction[2]*init->t +
				init->origin[2] + norm[2]*bias;
		
		//it works if I don't negate it?
		shadow.direction[0] = light->light_vector[0];
		shadow.direction[1] = light->light_vector[1];
		shadow.direction[2] = light->light_vector[2];
		
		if (ray_triangle_intersect(&shadow,
			&t, 0, 0,
			objs[cur_obj]->polys->m[0][cur_poly],
			objs[cur_obj]->polys->m[1][cur_poly],
			objs[cur_obj]->polys->m[2][cur_poly],
			objs[cur_obj]->polys->m[0][cur_poly+1],
			objs[cur_obj]->polys->m[1][cur_poly+1],
			objs[cur_obj]->polys->m[2][cur_poly+1],
			objs[cur_obj]->polys->m[0][cur_poly+2],
			objs[cur_obj]->polys->m[1][cur_poly+2],
			objs[cur_obj]->polys->m[2][cur_poly+2])) {
			
			if (objs[cur_obj]->behavior !=
				REFLECTION_AND_REFRACTION)
				return 1;
		}
	}//end poly loop
	}//end obj loop
	
	return 0;
}

void clamp(float lo, float hi, float *val) {
	*val = *val > 1 ? 1 : *val;
	*val = *val < -1 ? -1 : *val;
}

void free_ray(struct Ray *r) {
	free(r);
}

