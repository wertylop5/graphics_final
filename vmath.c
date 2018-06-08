#include"include/vmath.h"

float dot_product(float *a, float *b) {
	return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

void cross_product(float *out, float *a, float *b) {
	out[0] = a[1]*b[2] - a[2]*b[1];
	out[1] = a[2]*b[0] - a[0]*b[2];
	out[2] = a[0]*b[1] - a[1]*b[0];
}

void cross_productd(double *out, double *a, double *b) {
	out[0] = a[1]*b[2] - a[2]*b[1];
	out[1] = a[2]*b[0] - a[0]*b[2];
	out[2] = a[0]*b[1] - a[1]*b[0];
}

float triple_product(float *a, float *b, float *c) {
	float temp[3];
	cross_product(temp, a, b);
	return dot_product(temp, c);
}

void add_vectors(float *out, float *a, float *b) {
	out[0] = a[0] + b[0];
	out[1] = a[1] + b[1];
	out[2] = a[2] + b[2];
}

void subtract_vectors(float *out, float *a, float *b) {
	out[0] = a[0] - b[0];
	out[1] = a[1] - b[1];
	out[2] = a[2] - b[2];
}

void normalize(float *v) {
	float mag = sqrtf(powf(v[0], 2) + powf(v[1], 2) + powf(v[2], 2));
	
	v[0] /= mag;
	v[1] /= mag;
	v[2] /= mag;
}

void find_norm(struct Matrix *m, int p1, int p2, int p3,
		float *norm_out) {
	//create vectors
	double tempA[] = {
		m->m[0][p2] - m->m[0][p1],
		m->m[1][p2] - m->m[1][p1],
		m->m[2][p2] - m->m[2][p1],
	};
	
	double tempB[] = {
		m->m[0][p3] - m->m[0][p1],
		m->m[1][p3] - m->m[1][p1],
		m->m[2][p3] - m->m[2][p1]
	};
	
	//cross product
	/*
	norm_out[0] = tempA[1]*tempB[2] - tempA[2]*tempB[1];
	norm_out[1] = tempA[2]*tempB[0] - tempA[0]*tempB[2];
	norm_out[2] = tempA[0]*tempB[1] - tempA[1]*tempB[0];
	*/
	cross_product(norm_out, tempA, tempB);
}

void find_normn(float *norm_out,
		float x1, float y1, float z1,
		float x2, float y2, float z2,
		float x3, float y3, float z3) {
	float a[] = {x1, y1, z1};
	float b[] = {x2, y2, z2};
	float c[] = {x3, y3, z3};
	
	//create vectors
	float tempA[3], tempB[3];
	subtract_vectors(tempA, b, a);
	subtract_vectors(tempB, c, a);
	
	cross_product(norm_out, tempA, tempB);
}

