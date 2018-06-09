/*
 * compiler.h
 *
 * Contains functionality for parsing MDL commands
 * */

#ifndef COMPILER_H
#define COMPILER_H

void pass_one(int *tot_frames, char *anim_name, int max_len);

struct vary_node** pass_two(int tot_frames);

void process_knobs(struct vary_node **knobs, int cur_frame);

#endif
