#ifndef POINT_3D_H_
#define POINT_3D_H_

#include <stdio.h> /* printf, sprintf, fprintf */
#include <stdlib.h> /* malloc, calloc */
#include <string.h>
#include <stdbool.h>
#include "numerics.h"

#define MAX_PT_STR_LEN 112
#define MAX_COORD_STR_LEN 32

typedef struct point3d Point3D;

struct point3d
{
	float x;
	float y;
	float z;
} __attribute__((packed));

extern const Point3D UNIT_X;
extern const Point3D UNIT_Y;
extern const Point3D UNIT_Z;

void pt_init(Point3D *pt);

void pt_reset(Point3D *pt);

void pt_copy(const Point3D *pt, Point3D *pt_cp);

float pt_norm_sq(const Point3D *pt);

void pt_normalize(Point3D *pt);

Point3D pt_add(const Point3D *pt_1, const Point3D *pt_2);

Point3D pt_neg(const Point3D *pt);

Point3D pt_subtr(const Point3D *pt_1, const Point3D *pt_2);

Point3D pt_mult(float scale, const Point3D *pt);

void pt_cross(Point3D *c, const Point3D *a, const Point3D *b);

bool pt_great_than(Point3D *pt_1, Point3D *pt_2, float eps);

bool pt_less_than(Point3D *pt_1, Point3D *pt_2, float eps);

bool pt_great_than_or_eq(Point3D *pt_1, Point3D *pt_2, float eps);

bool pt_less_than_or_eq(Point3D *pt_1, Point3D *pt_2, float eps);

bool pt_equal(Point3D *pt_1, Point3D *pt_2, float eps);

char *pt_to_str(Point3D pt);

void print_pt(Point3D pt);

#endif /* POINT_3D_H_ */

