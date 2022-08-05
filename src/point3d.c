#include "point3d.h"

const Point3D UNIT_X = {1.0, 0.0, 0.0};
const Point3D UNIT_Y = {0.0, 1.0, 0.0};
const Point3D UNIT_Z = {0.0, 0.0, 1.0};

void pt_init(Point3D *pt)
{
	if (pt)
	{
		pt->x = 0.0;
		pt->y = 0.0;
		pt->z = 0.0;
	}
}


void pt_reset(Point3D *pt)
{
	if (pt)
	{
		pt->x = 0.0;
		pt->y = 0.0;
		pt->z = 0.0;
	}	
}


void pt_copy(const Point3D *pt, Point3D *pt_cp)
{
	pt_cp->x = pt->x;
	pt_cp->y = pt->y;
	pt_cp->z = pt->z;
}


float pt_norm_sq(const Point3D *pt)
{
	return pt->x * pt->x + pt->y * pt->y + pt->z * pt->z;
}

void pt_normalize(Point3D *pt)
{
	float inv_norm = 1.0 / sqrtf(pt_norm_sq(pt)); /* expensive */
	pt->x *= inv_norm;
	pt->y *= inv_norm;
	pt->z *= inv_norm;
}

Point3D pt_add(const Point3D *pt_1, const Point3D *pt_2)
{
	Point3D result;
	result.x = pt_1->x + pt_2->x;
	result.y = pt_1->y + pt_2->y;
	result.z = pt_1->z + pt_2->z;
	return result;
}


Point3D pt_neg(const Point3D *pt)
{
	Point3D result;
	result.x = -1.0 * pt->x;
	result.y = -1.0 * pt->y;
	result.z = -1.0 * pt->z;
	return result;
}


Point3D pt_subtr(const Point3D *pt_1, const Point3D *pt_2)
{
	Point3D pt_2_neg = pt_neg(pt_2);
	return pt_add(pt_1, &pt_2_neg);
}


Point3D pt_mult(float scale, const Point3D *pt)
{
	Point3D result;
	result.x = scale * pt->x;
	result.y = scale * pt->y;
	result.z = scale * pt->z;
	return result;
}

void pt_cross(Point3D *c, const Point3D *a, const Point3D *b)
{
	c->x = a->y * b->z - a->z * b->y;
	c->y = a->z * b->x - a->x * b->z;
	c->z = a->x * b->y - a->y * b->x;
}

// NOTE: all point comparisons use epsilon = 1.0E-6 implicitly
// FIXME: SHOULD NOT USE COMBINATION ORDER RELATION, ELEMENT-WISE EQUALITY VERSUS
// 		  NORM_SQ FOR INEQUALITY!!!
bool pt_great_than(Point3D *pt_1, Point3D *pt_2, float eps)
{
	return flt_great_than(pt_norm_sq(pt_1), pt_norm_sq(pt_2), eps);
}


bool pt_less_than(Point3D *pt_1, Point3D *pt_2, float eps)
{
	return pt_great_than(pt_2, pt_1, eps);
}

// non-exclusive or
bool pt_great_than_or_eq(Point3D *pt_1, Point3D *pt_2, float eps)
{
	return  pt_great_than(pt_1, pt_2, eps) || pt_equal(pt_1, pt_2, eps);
}

// non-exclusive or
bool pt_less_than_or_eq(Point3D *pt_1, Point3D *pt_2, float eps)
{
	return  pt_less_than(pt_1, pt_2, eps) || pt_equal(pt_1, pt_2, eps);
}


bool pt_equal(Point3D *pt_1, Point3D *pt_2, float eps)
{
		return (flt_near_eq(pt_1->x, pt_2->x, eps))
			&& (flt_near_eq(pt_1->y, pt_2->y, eps))
			&& (flt_near_eq(pt_1->z, pt_2->z, eps));
}

/* 
 *
 * NOTE: have to free returned value later
 * courtesy of David Rankin's answer to this stack exchange post:
 * 		https://stackoverflow.com/questions/29762048/c-structure-to-string 
 *
 */
char *pt_to_str(Point3D pt)
{
	size_t len = 0;
	len = snprintf(NULL, len, "%.2f, %.2f, %.2f",
				   pt.x, pt.y, pt.z);
	char *result = calloc(1, sizeof(*result * len + 1));
	if (!result)
	{
		fprintf(stderr,
				"%s() error: virtual memory allocation failed.\n",
				__func__);
	}
	if (snprintf(result, len + 1, "%.2f, %.2f, %.2f",
				 pt.x, pt.y, pt.z) > len + 1)
	{
		fprintf(stderr,
				"%s() error: snprintf returned truncated result.\n",
				__func__);
		return NULL;
	}
	return result;
}

void print_pt(Point3D pt)
{
	char *pt_str = pt_to_str(pt);
	printf("%s\n", pt_str);
	if (pt_str) free(pt_str);
}

