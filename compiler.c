#include"include/compiler.h"

#include "compiler/parser.h"
#include "compiler/symtab.h"
#include "y.tab.h"

void pass_one(int *tot_frames, char *anim_name, int max_len) {
	char vary_found = 0;
	int x = 0;
	while ( op[x].opcode != 0 ) {
	switch(op[x].opcode) {
		case FRAMES:
			(*tot_frames) = op[x].op.frames.num_frames;
			//printf("frames found: %d\n", *tot_frames);
		break;
		
		case VARY:
			vary_found = 1;
			//printf("vary found\n");
		break;
		
		case BASENAME:
			strncpy(anim_name, op[x].op.basename.p->name,
					max_len);
			//printf("basename found: %s\n", anim_name);
		break;
	}
	x++;
	}
	
	if (vary_found && *tot_frames == -1) {
		fprintf(stderr, "ERROR: vary command found, but number of frames not specified\n");
		exit(1);
	}
}

struct vary_node** pass_two(int tot_frames) {
	struct vary_node **res = (struct vary_node **)malloc(
		tot_frames * sizeof(struct vary_node *));
	
	//keep tracks of the tail of the linked lists
	struct vary_node* tails[tot_frames];
	memset(tails, 0, sizeof(tails));
	
	int x;
	for (x = 0; x < tot_frames; x++) {
		res[x] = 0;
	}
	
	x = 0;
	while ( op[x].opcode != 0 ) {
	switch(op[x].opcode) {
	case VARY:
	{
		char *knob = op[x].op.vary.p->name;
		//printf("Accessing knob: %s, with len %lu\n", knob, strlen(knob));
		
		int cur_frame;
		float cur;	//cuz frames are floats?
		float step = (op[x].op.vary.end_val - op[x].op.vary.start_val) /
			(op[x].op.vary.end_frame - op[x].op.vary.start_frame);
		for (cur = op[x].op.vary.start_frame;
				cur <= op[x].op.vary.end_frame;
				cur++) {
			cur_frame = (int)cur;
			/*
			 * If null, create a element and edit it directly.
			 * 
			 * If not null, create new element and 
			 * have prev->next point to new element
			 * */
			if (res[cur_frame] == 0) {
				res[cur_frame] = (struct vary_node *)malloc(sizeof(struct vary_node));
				strncpy(res[cur_frame]->name, knob, strlen(knob));
				res[cur_frame]->value = op[x].op.vary.start_val+
					(step*(cur-op[x].op.vary.start_frame));
				res[cur_frame]->next = 0;
				tails[cur_frame] = res[cur_frame];
			}
			else {
				struct vary_node *temp = (struct vary_node *)malloc(sizeof(struct vary_node));
				strncpy(temp->name, knob, strlen(knob));
				temp->value = op[x].op.vary.start_val+
					(step*(cur-op[x].op.vary.start_frame));
				temp->next = 0;
				
				tails[cur_frame]->next = temp;
				tails[cur_frame] = temp;
			}
		}
	}
	break;
	}
	x++;
	}
	
	x = 0;
	for (; x < tot_frames; x++) {
		//printf("Frame %d\n", x);
		if (res[x] != 0) {
			struct vary_node const * temp = res[x];
			while (temp != 0) {
				//printf("knob %s with value %f\n", temp->name, temp->value);
				temp = temp->next;
			}
		}
	}
	
	return res;
}

void process_knobs(struct vary_node **knobs, int cur_frame) {
	struct vary_node *temp = knobs[cur_frame];
	while (temp != 0) {
		//printf("on knob: %s\n", temp->name);
		set_value(lookup_symbol(temp->name), temp->value);
		temp = temp->next;
	}
}
