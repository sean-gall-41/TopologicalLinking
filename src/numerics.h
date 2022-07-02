#ifndef NUMERICS_H_
#define NUMERICS_H_

#include <string.h> /* memcpy */
#include <stdbool.h>
#include <math.h> /* fabs */
#include <Random123/threefry.h>
#include <Random123/u01fixedpt.h>

void flt_to_bytes(unsigned char byte_buff[4], float val);

bool flt_near_eq(float a, float b, float eps);

int rand_int(threefry2x32_ctr_t *ctr, threefry2x32_key_t *key, 
	int a, int b);

float rand_flt(threefry2x32_ctr_t *ctr, threefry2x32_key_t *key,
	float a, float b);

bool flt_great_than(float a, float b, float eps);

bool flt_less_than(float a, float b, float eps);


#endif /* NUMERICS_H_ */
