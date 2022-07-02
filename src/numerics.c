#include "numerics.h"

void flt_to_bytes(unsigned char byte_buff[4], float val)
{
	union
	{
		float var;
		unsigned char bytes[4];
	} flt_union;
	flt_union.var = val;
	memcpy(byte_buff, flt_union.bytes, 4);
}

bool flt_near_eq(float a, float b, float eps)
{
    return fabs(a - b) <= ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * eps);
}

int rand_int(threefry2x32_ctr_t *ctr, threefry2x32_key_t *key, 
	int a, int b)
{
	ctr->v[0]++;
	threefry2x32_ctr_t rand = threefry2x32(*ctr, *key);
	return rand.v[0] % (b - a) + a;
}

float rand_flt(threefry2x32_ctr_t *ctr, threefry2x32_key_t *key,
	float a, float b)
{
	ctr->v[0]++; // increment counter	
	threefry2x32_ctr_t rand = threefry2x32(*ctr, *key);
	float return_val = u01fixedpt_closed_open_32_float(rand.v[0]);
	if (a == 0.0 && b == 1.0) return return_val;	
	return (b - a) * return_val + a;	
}

bool flt_great_than(float a, float b, float eps)
{
    return (a - b) > ( (fabs(a) < fabs(b) ? fabs(b) : fabs(a)) * eps);
}

bool flt_less_than(float a, float b, float eps)
{
	return flt_great_than(b, a, eps);
}

