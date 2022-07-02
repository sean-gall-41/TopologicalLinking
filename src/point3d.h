#ifndef POINT_3D_H_
#define POINT_3D_H_

#include <stdio.h> /* printf, sprintf, fprintf */
#include <stdlib.h> /* malloc, calloc */
#include <string.h>
#include <stdbool.h>
#include "numerics.h"

#define MAX_PT_STR_LEN 112
#define MAX_COORD_STR_LEN 32

typedef struct 
{
	float x;
	float y;
	float z;
} Point3D;

void pt_init(Point3D *pt);

void pt_reset(Point3D *pt);

void pt_copy(Point3D *pt, Point3D *pt_cp);

float pt_norm_sq(Point3D *pt);

Point3D pt_add(Point3D *pt_1, Point3D *pt_2);

Point3D pt_neg(Point3D *pt);

Point3D pt_subtr(Point3D *pt_1, Point3D *pt_2);

Point3D pt_mult(float scale, Point3D *pt);

bool pt_great_than(Point3D *pt_1, Point3D *pt_2, float eps);

bool pt_less_than(Point3D *pt_1, Point3D *pt_2, float eps);

bool pt_great_than_or_eq(Point3D *pt_1, Point3D *pt_2, float eps);

bool pt_less_than_or_eq(Point3D *pt_1, Point3D *pt_2, float eps);

bool pt_equal(Point3D *pt_1, Point3D *pt_2, float eps);

char *pt_to_str(Point3D pt);

void print_pt(Point3D pt);

#endif /* POINT_3D_H_ */

