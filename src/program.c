#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <GL/gl.h>

#include "program.h"
#include "util.h"

/* good answer and reminder on how extern works: https://stackoverflow.com/a/496476 */ 
#define DATA_DEF(x)                                              \
	extern const uint8_t _binary_shaders_ ## x ## _glsl_start[]; \
	extern const uint8_t _binary_shaders_ ## x ## _glsl_end[]; 

#define SHADER(x)                                 \
	{                                             \
	  .buf = _binary_shaders_ ## x ## _glsl_start \
	, .end = _binary_shaders_ ## x ## _glsl_end   \
	}

// inline data definitions
DATA_DEF(vertex)
DATA_DEF(fragment)

struct shader
{
	const uint8_t *buf;
	const uint8_t *end;
	GLuint id;
};

enum loc_type
{
	UNIFORM,
	ATTRIBUTE
};

struct loc
{
	const char *name;
	enum loc_type type;
	GLint id;
};

static struct loc loc_bkgd[] = {
	[LOC_BKGD_VERTEX]  = { "vertex",  ATTRIBUTE },
	[LOC_BKGD_TEXTURE] = { "texture", ATTRIBUTE }
};

// programs
enum
{
	BKGD
};

static struct program
{
	struct
	{
		struct shader vert;
		struct shader frag;
	} shader;
	struct loc *loc;
	size_t nloc;
	GLuint id;
}
programs[] = {
	[BKGD] = {
			 .shader.vert = SHADER(vertex),
			 .shader.frag = SHADER(fragment),
			 .loc		  = loc_bkgd,
			 .nloc		  = NELEM(loc_bkgd)
	}
};

static void check_compile(GLuint shader)
{
	GLint length;

	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

	if (length <= 1) return;

	GLchar *log = calloc(length, sizeof(GLchar));
	glGetShaderInfoLog(shader, length, NULL, log);
	fprintf(stderr, "glCompileShader failed: \n%s\n", log);
	free(log);
}

static void check_link(GLuint program)
{
	GLint status, length;

	glGetProgramiv(program, GL_LINK_STATUS, &status);

	if (status != GL_FALSE) return;
	
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
	GLchar *log = calloc(length, sizeof(GLchar));
	glGetProgramInfoLog(program, length, NULL, log);
	fprintf(stderr, "glLinkProgram failed: %s\n", log);
	free(log);
}

static void create_shader(struct shader *shader, GLenum type)
{
	const GLchar *buf = (const GLchar *)shader->buf;
	GLint len = shader->end - shader->buf;

	shader->id = glCreateShader(type);
	glShaderSource(shader->id, 1, &buf, &len);
	glCompileShader(shader->id);

	check_compile(shader->id);
}

static void program_init(struct program *program)
{
	struct shader *vert = &program->shader.vert;
	struct shader *frag = &program->shader.frag;

	create_shader(vert, GL_VERTEX_SHADER);
	create_shader(frag, GL_FRAGMENT_SHADER);

	program->id = glCreateProgram();

	glAttachShader(program->id, vert->id);
	glAttachShader(program->id, frag->id);

	glLinkProgram(program->id);
	check_link(program->id);

	glDetachShader(program->id, vert->id);
	glDetachShader(program->id, frag->id);

	glDeleteShader(vert->id);
	glDeleteShader(frag->id);

	FOREACH_NELEM(program->loc, program->nloc, l)
	{
		switch (l->type)
		{
			case UNIFORM:
				l->id = glGetUniformLocation(program->id, l->name);
				break;

			case ATTRIBUTE:
				l->id = glGetAttribLocation(program->id, l->name);
				break;
		}
	}
}

// NOTE: explicitly calling background program, will use struct loop when adding progs later
void programs_init(void)
{
	program_init(&programs[BKGD]);
}

void program_bkgd_use(void)
{
	glUseProgram(programs[BKGD].id);
	glUniform1i(glGetUniformLocation(programs[BKGD].id, "tex"), 0);
}

GLint program_bkgd_loc(const enum LocBkgd index)
{
	return  loc_bkgd[index].id;
}

