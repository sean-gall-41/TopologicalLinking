#include <gdk/gdk.h>
#include <GL/gl.h>

#include "program.h"

static GLuint texture;
static GLuint vao, vbo;

struct vertex
{
	float x;
	float y;
	float u;
	float v;
} __attribute__((packed));

void background_set_window(int width, int height)
{
	float wd = (float)width / 16;
	float ht = (float)height / 16;

	/* background quad is made of four vertices:
	 *
	 *              3 ---- 2
	 *              |      |
	 *              |      |
	 *              0 ---- 1
	 *
	 * each background vertex is mapped from coord space to texture space,
	 * as in the following diagram:
	 *    
	 *                 |
	 *                 |
	 *                 |                          o (0, ht)           o (wd, ht)
	 * (-1, 1) o       |      o (1, 1)            |
	 *                 |                          |
	 *                 |                          |
	 *    -------------+--------------      ==>   |
	 *                 |                          | 
	 *                 |                          | 
	 * (-1, -1) o      |      o (1, -1)           | 
	 *                 |                          |
	 *                 |                          o-------------------o---
	 *                 |                          (0, 0)              (wd, 0)
	 * 
	 */
	struct vertex vertex[4] = {
		{ -1, -1,  0, 0  }, // btm left
		{  1, -1, wd, 0  }, // btm right
		{  1,  1, wd, ht }, // top right
		{ -1,  1,  0, ht }  // top left (busta)
	};

	// get the ids (locations) of the coord vertices and texture vertices
	GLint loc_vertex  = program_bkgd_loc(LOC_BKGD_VERTEX);
	GLint loc_texture = program_bkgd_loc(LOC_BKGD_TEXTURE);

	glBindVertexArray(vao); /* bind vertex array object to vao (prev returned from glGenVertexArrays) */

	glEnableVertexAttribArray(loc_vertex); /* enable vertex attrib array for the coord vertices */
	glEnableVertexAttribArray(loc_texture); /* do same as above but for texture vertices */

	glBindBuffer(GL_ARRAY_BUFFER, vbo); /* bind vbo to vertex attrib array */
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW); /* create vbo data store */

	// define attrib array formatting for coord vertices
	glVertexAttribPointer(loc_vertex, 2, GL_FLOAT, GL_FALSE,
						  sizeof(struct vertex),
						  (void *) offsetof(struct vertex, x));

	// define attrib array formatting for texture vertices
	glVertexAttribPointer(loc_texture, 2, GL_FLOAT, GL_FALSE,
						  sizeof(struct vertex),
						  (void *) offsetof(struct vertex, u));

	glBindVertexArray(0); /* remove existing vertex array object binding */
}

void background_init(void)
{
	// inline data declaration
	extern char _binary_textures_background_png_start[];
	extern char _binary_textures_background_png_end[];

	char *start = _binary_textures_background_png_start;
	size_t len = _binary_textures_background_png_end
			   - _binary_textures_background_png_start;

	GInputStream *stream;
	GdkPixbuf *pixbuf; 

	// create input stream from the inline data
	stream = g_memory_input_stream_new_from_data(start, len, NULL);

	// create pixel buffer from the input stream
	pixbuf = gdk_pixbuf_new_from_stream(stream, NULL, NULL);

	// destroy the stream
	g_object_unref(stream);

	// generate OpenGL texture from pixbuf
	// hack a bit by not accountring for pixbuf rowstride
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
				 gdk_pixbuf_get_width(pixbuf),
				 gdk_pixbuf_get_height(pixbuf), 0, GL_RGB, GL_UNSIGNED_BYTE,
				 gdk_pixbuf_get_pixels(pixbuf));

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	// generate an empty buffer
	glGenBuffers(1, &vbo);

	// generate an empty vertex array object
	glGenVertexArrays(1, &vao);
}

void background_draw(void)
{
	// index array with two ccw triangles:
	// 0-2-3 and 2-0-1 (see background_set_window diagram for vertex indices)
	static GLubyte index[6] = {
			0, 2, 3,
			2, 0, 1
	};

	program_bkgd_use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, index);
	glBindVertexArray(0);
}

