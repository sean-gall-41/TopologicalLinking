#include <GL/gl.h>

#include "program.h"
#include "util.h"
#include "point3d.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define NUM_CAPS 2
#define POLY_SIDES 8
#define TRI_PER_RECT 2

// for now, our model is a single cylinder, but we will generalize this to
// an array of cylinders whose two caps' planes' centers are the points we
// generate in our SAW algorithm

typedef struct texture texture;
typedef struct color color;
typedef struct vertex vertex;
typedef struct triangle triangle;
typedef struct rectangle rectangle;
typedef struct reg_poly reg_poly;
typedef struct cylinder cylinder;

struct texture
{
	float s;
	float t;
} __attribute__((packed));

struct color
{
	float r;
	float g;
	float b;
} __attribute__((packed));

struct vertex
{
	Point3D pos;
	Point3D normal;
	texture text_coord;
	//color color;
} __attribute__((packed));

struct triangle
{
	vertex vertices[3];
} __attribute__((packed));

struct rectangle
{
	Point3D normal;
	vertex vertices[4];
} __attribute__((packed));

/*
 * Description:
 *     Each regular polygon has:
 *         
 *         (2) a radius (of the circumscribed circle)
 *         (3) an array of triangles
 *     
 *     Took out the number of sides, as we will make this a global constant
 *
 */
struct reg_poly
{
	float radius;
	Point3D normal;
	vertex vertices[POLY_SIDES]; 
} __attribute__((packed));

/*
 * Description:
 *     Each cylinder has:
 *
 *         (1) NUM_CAPS number of polygonal caps (should *always* be 2)
 *         (2) POLY_SIDES number of rectangles
 *
 *     a cylinder has POLY_SIDES number of rectangles as each rectangle's length
 *     intersects a side of a cap.
 *
 */
struct cylinder
{
	reg_poly caps[NUM_CAPS]; 
	rectangle rectangles[POLY_SIDES]; 
} __attribute__((packed));

static GLuint vao, vbo;
static float matrix[16] = {0};

static struct
{
	int x;
	int y;
} pan;

void color_copy(const color *clr, color *clr_copy)
{
	clr_copy->r = clr->r;
	clr_copy->g = clr->g;
	clr_copy->b = clr->b;
}

static void vertex_copy(const vertex *vert, vertex *vert_copy)
{
	pt_copy(&(vert->pos), &(vert_copy->pos));
	pt_copy(&(vert->normal), &(vert_copy->normal));
	//color_copy(&(vert->color), &(vert_copy->color));
}

static void calc_unit_circle_vert_xy_plane(reg_poly *circle)
{ 
	circle->radius = 1.0;
	pt_copy(&UNIT_Z, &(circle->normal));
	float angle_step = 2.0 * M_PI / (float)POLY_SIDES;
	float angle;
	for (int i = 0; i < POLY_SIDES; i++)
	{
		angle = i * angle_step;
		
		float x_coord = cosf(angle);
		float y_coord = sinf(angle);
		float z_coord = 0.0;

		circle->vertices[i].pos.x = x_coord;
		circle->vertices[i].pos.y = y_coord;
		circle->vertices[i].pos.z = z_coord;

		circle->vertices[i].normal.x = 0.0;
		circle->vertices[i].normal.y = 0.0;
		circle->vertices[i].normal.z = 1.0;
		
		circle->vertices[i].text_coord.s = -x_coord * 0.5f + 0.5f;
		circle->vertices[i].text_coord.t = -y_coord * 0.5f + 0.5f;
	}
}

static void set_reg_poly_vertex_norms(Point3D *norm, reg_poly *poly)
{
	FOREACH(poly->vertices, vp)
	{
		pt_copy(norm, &(vp->normal));
	}
}

static void reg_poly_copy(const reg_poly *poly, reg_poly *poly_copy)
{
	poly_copy->radius = poly->radius;
	pt_copy(&(poly->normal), &(poly_copy->normal));
	for (int i = 0; i < POLY_SIDES; i++)
	{
		vertex_copy(&(poly->vertices[i]), &(poly_copy->vertices[i]));
	}
}

static void dilate_reg_poly_xy_plane(float scale, reg_poly *poly)
{
	poly->radius *= scale;
	FOREACH(poly->vertices, vi)
	{
		vi->pos.x *= scale;
		vi->pos.y *= scale;
	}
}

static void translate_poly(Point3D *dir, reg_poly *poly)
{
	for (int i = 0; i < POLY_SIDES; i++)
	{
		Point3D trans_pt = pt_add(dir, &(poly->vertices[i].pos));
		pt_copy(&trans_pt, &(poly->vertices[i].pos));
	}
}

static void translate_poly_z(float dist, reg_poly *poly)
{
	Point3D scaled_unit_z = pt_mult(dist, &UNIT_Z);
	translate_poly(&scaled_unit_z, poly);
}

static void cylinder_init_z(cylinder *cylinder, float radius, float length)
{
	// caps
	float cap_plane = length / 2;
	reg_poly circle;
	calc_unit_circle_vert_xy_plane(&circle);
	dilate_reg_poly_xy_plane(radius, &circle);
	for (int i = 0; i < NUM_CAPS; i++)
	{
		// radii and normals
		cylinder->caps[i].radius = radius; /* assign radius */
		Point3D cap_norm = pt_mult((float)(1 - 2 * i), &UNIT_Z); /* generate cap norm */
		pt_copy(&cap_norm, &(cylinder->caps[i].normal)); /* assign normal to cap */
		set_reg_poly_vertex_norms(&cap_norm, &(circle)); /* set the circle norms */

		// vertices
		reg_poly_copy(&circle, &(cylinder->caps[i])); /* texture coords taken care of here */
		translate_poly_z(cap_plane, &(cylinder->caps[i])); /* translate the cap by the scaled normal */
		cap_plane -= length; /* move cap plane to next location */

	}

	// rectangles
	for (int i = 0; i < POLY_SIDES; i++)
	{
		// vertex coords
		pt_copy(&(cylinder->caps[0].vertices[i].pos),
				&(cylinder->rectangles[i].vertices[0].pos));
		pt_copy(&(cylinder->caps[1].vertices[i].pos),
				&(cylinder->rectangles[i].vertices[1].pos));
		pt_copy(&(cylinder->caps[1].vertices[(i+1) % POLY_SIDES].pos),
				&(cylinder->rectangles[i].vertices[2].pos));
		pt_copy(&(cylinder->caps[0].vertices[(i+1) % POLY_SIDES].pos),
				&(cylinder->rectangles[i].vertices[3].pos));

		// vertex normals
		pt_copy(&(circle.vertices[i].normal),
				&(cylinder->rectangles[i].vertices[0].normal));
		pt_copy(&(circle.vertices[i].normal),
				&(cylinder->rectangles[i].vertices[1].normal));
		pt_copy(&(circle.vertices[(i+1) % POLY_SIDES].normal),
				&(cylinder->rectangles[i].vertices[2].normal));
		pt_copy(&(circle.vertices[(i+1) % POLY_SIDES].normal),
				&(cylinder->rectangles[i].vertices[3].normal));

		// vertex texture coords
		for (int j = 0; j < 4; j ++)
		{ 
			cylinder->rectangles[i].vertices[j].text_coord.s = (float)i / POLY_SIDES;
			cylinder->rectangles[i].vertices[j].text_coord.t = j % 2; /* alternating sequence of 0s and 1s */
		}

		// rectangle normal
		Point3D rect_unit_vec_u = pt_subtr(&(cylinder->rectangles[i].vertices[1].pos),
										   &(cylinder->rectangles[i].vertices[0].pos));

		Point3D rect_unit_vec_v = pt_subtr(&(cylinder->rectangles[i].vertices[2].pos),
										   &(cylinder->rectangles[i].vertices[1].pos));

		Point3D rect_norm;
		pt_cross(&rect_norm, &rect_unit_vec_u, &rect_unit_vec_v);
		pt_normalize(&rect_norm);
		pt_copy(&rect_norm, &(cylinder->rectangles[i].normal));
	}
}

static void calc_cylinder_triangles(triangle triangles[], cylinder *cylinder)
{
	// TODO: write :)
}

static void assemble_cylinder_interweaved_vertex_attribs(float vert_attribs[], cylinder *cylinder)
{
	// TODO: write :))
}

