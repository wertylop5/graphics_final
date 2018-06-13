#ifndef PARSER_H
#define PARSER_H

#include "symtab.h"
#include "../include/matrix.h"

#define MAX_COMMANDS 512
#define FRAME_DIR "frames/"

extern int lastop;

#define Ka 0
#define Kd 1
#define Ks 2


struct command
{
  int opcode;
  union
  {
    struct {
      SYMTAB *p;
      double c[4];
    } light;
    struct {
      double c[4];
    } ambient;
    struct {
      SYMTAB *p;
      /* each triple holds ka kd and ks for red green and blue
         respectively */
    } constants;
    struct {
      SYMTAB *p;
    } save_coordinate_system;
    struct {
      double eye[4],aim[4];
    } camera;
    struct {
      SYMTAB *constants;
      double d[4];
      double r;
      SYMTAB *cs;
      SYMTAB *behavior;
    } sphere;
    struct {
      SYMTAB *constants;
      double d0[3];
      double d1[3];
      double d2[3];
      double d3[3];
      SYMTAB *p;
      SYMTAB *cs;
    } texture;
    struct {
      SYMTAB *constants;
      double d[4];
      double r0,r1;
      SYMTAB *cs;
      SYMTAB *behavior;
    } torus;
    struct {
      SYMTAB *constants;
      double d0[4],d1[4];
      SYMTAB *cs;
    } box;
    struct {
      SYMTAB *constants;
      double p0[4],p1[4];
      SYMTAB *cs0,*cs1;
    } line;
    struct {
      SYMTAB *constants;
      char name[255];
      SYMTAB *cs;
    } mesh;
    struct {
      SYMTAB *p;
      double val;
    } set;
    struct {
      double d[4];
      SYMTAB *p;
    } move;
    struct {
      double d[4];
      SYMTAB *p;
    } scale;
    struct {
      double axis;
      double degrees;
      SYMTAB *p;
    } rotate;
    struct {
      SYMTAB *p;
    } basename;
    struct {
      SYMTAB *p;
    } save_knobs;
    struct {
      float start_frame, end_frame;
      SYMTAB *knob_list0;
      SYMTAB *knob_list1;
    } tween;
    struct {
      float num_frames;
    }frames;
    struct {
      SYMTAB *p;
      float start_frame, end_frame, start_val, end_val;
    } vary;
    struct {
      SYMTAB *p;
    } save;
    struct {
      SYMTAB *p;
    } shading;
    struct {
      float value;
    } setknobs;
    struct {
      float value;
    } focal;
  } op;
};

struct vary_node {
	char name[128];
	float value;
	struct vary_node *next;
};


extern struct command op[MAX_COMMANDS];

void pass_one(int *tot_frames, char *anim_name, int max_len);
struct vary_node** pass_two(int tot_frames);
void process_knobs(struct vary_node **knobs, int cur_frame);
void print_pcode();
void my_main();

#endif
