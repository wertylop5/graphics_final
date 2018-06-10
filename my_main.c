#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "include/draw.h"
#include "include/rcs.h"
#include "include/shapes.h"
#include "include/output.h"
#include "include/raytrace.h"
#include "include/compiler.h"
#include "include/uthash.h"

#include "compiler/parser.h"
#include "y.tab.h"

void my_main() {
	//environment vars
	int tot_frames = -1;	//if this still = -1, then user doesn't want animation
	char anim_name[128];
	strncpy(anim_name, "default", sizeof(anim_name));
	
	//look for animation commands
	pass_one(&tot_frames, anim_name, 128);
	struct vary_node **knobs;
	if (tot_frames > 0) {
		knobs = pass_two(tot_frames);
	}
	
	//globals
	struct Rcs_stack *s;
	//float view_vect[] = {0, 0, 1};
	struct Object *objs[100];
	struct Light *lights[10];
	int obj_count = 0, light_count = 0;
	
	Frame f;
	zbuffer z;
	struct Pixel pixel;
	//float aReflect[3];
	//float dReflect[3];
	//float sReflect[3];
	//float step = 15;
	//float theta;
	/*
	aReflect[RED] = 0.1;
	aReflect[GREEN] = 0.1;
	aReflect[BLUE] = 0.1;

	dReflect[RED] = 0.5;
	dReflect[GREEN] = 0.5;
	dReflect[BLUE] = 0.5;

	sReflect[RED] = 0.5;
	sReflect[GREEN] = 0.5;
	sReflect[BLUE] = 0.5;
	*/
	clear(f, z);
	pixel_color(&pixel, 0, 0, 0);
		
	int cur_frame;
	char frame_name[256];
	if (tot_frames == -1) tot_frames = 1;
	for (cur_frame = 0; cur_frame < tot_frames; cur_frame++) {//frame loop
	obj_count = light_count = 0;
	s = new_rcs_stack(3);
	lights[0] = new_light(0, 120, 0, 0, 255, 0, -1, 0, -1);
	light_count++;
	clear(f, z);
	
	//printf("frame %d\n", cur_frame);
	if (tot_frames > 1) {
		process_knobs(knobs, cur_frame);
	}
	int x = 0;
	while ( op[x].opcode != 0 ) {		//op loop
	switch(op[x].opcode) {
		case PUSH:
			push_rcs(s);
		break;
			
		case POP:
			pop_rcs(s);
		break;
		
		case MOVE:
		{
			double *temp = op[x].op.move.d;
			struct Matrix *t = move(
				temp[0] * (op[x].op.move.p ?
					op[x].op.move.p->s.value :
					1),
				temp[1] * (op[x].op.move.p ?
					op[x].op.move.p->s.value :
					1),
				temp[2] * (op[x].op.move.p ?
					op[x].op.move.p->s.value :
					1));
			
			matrix_mult(peek(s), t);
			free_matrix(peek(s));
			s->stack[s->top] = copy_matrix(t);
			
			free_matrix(t);
		break;
		}
		
		case SCALE:
		{
			double *temp = op[x].op.scale.d;
			struct Matrix *t = scale(
				temp[0] * (op[x].op.scale.p ?
					op[x].op.scale.p->s.value :
					1),
				temp[1] * (op[x].op.scale.p ?
					op[x].op.scale.p->s.value :
					1),
				temp[2] * (op[x].op.scale.p ?
					op[x].op.scale.p->s.value :
					1));
			
			matrix_mult(peek(s), t);
			free_matrix(peek(s));
			s->stack[s->top] = copy_matrix(t);
			
			free_matrix(t);
		break;
		}
		
		case ROTATE:
		{
			//why is axis defined as a double lmao
			struct Matrix *t = rotate(
				op[x].op.rotate.axis,
				op[x].op.rotate.degrees *
			       		(op[x].op.rotate.p ?
					op[x].op.rotate.p->s.value :
					1) 
				);
			
			matrix_mult(peek(s), t);
			free_matrix(peek(s));
			s->stack[s->top] = copy_matrix(t);
			
			free_matrix(t);
		break;
		}
			
		case BOX:
		{
			double *temp = op[x].op.box.d0;
			double *temp2 = op[x].op.box.d1;
			struct Matrix *p = new_matrix(4, 1);
			
			add_cube(p, temp[0], temp[1], temp[2],
				temp2[0], temp2[1], temp2[2]);
			matrix_mult(peek(s), p);
			objs[obj_count++] = new_object(
				p,
				1, 1, 1,
				AMBIENT_ONLY);
			
			free_matrix(p);
		break;
		}
		
		case SPHERE:
		{
			double *temp = op[x].op.sphere.d;
			struct Matrix *p = new_matrix(4, 1);
			
			add_sphere(p, temp[0], temp[1], temp[2],
				op[x].op.sphere.r, 12);
			matrix_mult(peek(s), p);
			objs[obj_count++] = new_object(
				p,
				1, 1, 1,
				DIFFUSE_AND_GLOSSY);
			
			free_matrix(p);
		break;
		}
		
		case TORUS:
		{
			double *temp = op[x].op.torus.d;
			struct Matrix *p = new_matrix(4, 1);
					
			add_torus(p, temp[0], temp[1], temp[2],
				op[x].op.torus.r0,
				op[x].op.torus.r1,
				15);
			matrix_mult(peek(s), p);
			objs[obj_count++] = new_object(
				p,
				1, 1, 1,
				DIFFUSE_AND_GLOSSY);
			
			free_matrix(p);
		break;
		}
		
		case LINE:
		{
			double *temp  = op[x].op.line.p0;
			double *temp2 = op[x].op.line.p1;
			struct Matrix *e = new_matrix(4, 1);
			
			push_edge(e, temp[0], temp[1], temp[2],
				temp2[0], temp2[1], temp2[2]);
			matrix_mult(peek(s), e);
			draw_lines(f, z, e, &pixel);
			free_matrix(e);
		break;
		}
			
		case SAVE:
			write_to_file(f);
			save_png(f, op[x].op.save.p->name);
		break;
		
		case DISPLAY:
			render(f, objs, lights,
				obj_count, light_count);
			
			display(f);
		break;
		
		};
	x++;
	}//end op loop
	if (tot_frames > 1) {
		//save the frame
		memset(frame_name, 0, sizeof(frame_name));
		snprintf(frame_name, sizeof(frame_name), "%s%04d%s.png", FRAME_DIR, cur_frame, anim_name);
		//printf("%s\n", frame_name);
		save_png(f, frame_name);
	}
	
	int temp_counter;
	for (temp_counter = 0; temp_counter < light_count;
					temp_counter++) {
			if (lights[temp_counter] == 0) break;
			free_light(lights[temp_counter]);
	}
	free_stack(s);
	for (temp_counter = 0; temp_counter < obj_count;
					temp_counter++) {
			if (objs[temp_counter] == 0) break;
			free_object(objs[temp_counter]);
	}
	}//end frame loop
	
	if (tot_frames > 1) {
		save_anim(anim_name, FRAME_DIR);
		view_anim(anim_name, FRAME_DIR);
	}
}
