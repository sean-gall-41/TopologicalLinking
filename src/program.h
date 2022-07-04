#ifndef PROGRAM_H_

enum LocBkgd
{
	LOC_BKGD_VERTEX,
	LOC_BKGD_TEXTURE
};

GLint program_bkgd_loc(const enum LocBkgd);

void programs_init(void);
void program_bkgd_use(void);

#define PROGRAM_H_
#endif /* PROGRAM_H_ */

