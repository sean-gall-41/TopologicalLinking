/*******************************************************************************
***
***     Author: Tyler Barrus
***     email:  barrust@gmail.com
***
***     Version: 0.2.0
***
***     License: MIT 2016
***
*******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "set.h"

#define MAX_FULLNESS_PERCENT 0.25       /* arbitrary */

/* PRIVATE FUNCTIONS */
static uint64_t __default_hash(const Point3D *key);
static int __get_index(PointSet *set, const Point3D *key, uint64_t hash, uint64_t *index);
static int __assign_node(PointSet *set, const Point3D *key, uint64_t hash, uint64_t index);
static void __free_index(PointSet *set, uint64_t index);
static int __set_contains(PointSet *set, const Point3D *key, uint64_t hash);
static int __set_add(PointSet *set, const Point3D *key, uint64_t hash);
static void __relayout_nodes(PointSet *set, uint64_t start, short end_on_null);

/*******************************************************************************
                             FUNCTION DEFINITIONS
*******************************************************************************/

int set_init_alt(PointSet *set, uint64_t num_els, set_hash_function hash)
{
    set->nodes = (point_set_node**) malloc(num_els * sizeof(point_set_node*));
    if (set->nodes == NULL) return SET_MALLOC_ERROR;
    set->number_nodes = num_els;
    for (uint64_t i = 0; i < set->number_nodes; ++i) set->nodes[i] = NULL;
    set->used_nodes = 0;
    set->hash_function = (hash == NULL) ? &__default_hash : hash;
    return SET_TRUE;
}

int set_clear(PointSet *set)
{
    for(uint64_t i = 0; i < set->number_nodes; ++i)
	{
        if (set->nodes[i] != NULL) __free_index(set, i);
    }
    set->used_nodes = 0;
    return SET_TRUE;
}

int set_destroy(PointSet *set)
{
    set_clear(set);
    free(set->nodes);
    set->number_nodes = 0;
    set->used_nodes = 0;
    set->hash_function = NULL;
    return SET_TRUE;
}

int set_add(PointSet *set, const Point3D *key)
{
    uint64_t hash = set->hash_function(key);
    return __set_add(set, key, hash);
}

int set_remove(PointSet *set, const Point3D *key)
{
    uint64_t index, hash = set->hash_function(key);
    int pos = __get_index(set, key, hash, &index);
    if (pos != SET_TRUE) return pos;
    // remove this node
    __free_index(set, index);
    // re-layout nodes
    __relayout_nodes(set, index, 0);
    --set->used_nodes;
    return SET_TRUE;
}

int set_contains(PointSet *set, const Point3D *key)
{
    uint64_t index, hash = set->hash_function(key);
    return __get_index(set, key, hash, &index);
}

uint64_t set_length(PointSet *set)
{
    return set->used_nodes;
}

int set_union(PointSet *res, PointSet *s1, PointSet *s2)
{
    if (res->used_nodes != 0) return SET_OCCUPIED_ERROR;
    // loop over both s1 and s2 and get keys and insert them into res
    for (uint64_t i = 0; i < s1->number_nodes; ++i)
	{
        if (s1->nodes[i] != NULL)
		{
            __set_add(res, s1->nodes[i]->_key, s1->nodes[i]->_hash);
        }
    }
    for (uint64_t i = 0; i < s2->number_nodes; ++i)
	{
        if (s2->nodes[i] != NULL)
		{
            __set_add(res, s2->nodes[i]->_key, s2->nodes[i]->_hash);
        }
    }
    return SET_TRUE;
}

int set_intersection(PointSet *res, PointSet *s1, PointSet *s2)
{
    if (res->used_nodes != 0) return SET_OCCUPIED_ERROR;
    // loop over both one of s1 and s2: get keys, check the other, and insert them into res if it is
    for (uint64_t i = 0; i < s1->number_nodes; ++i)
	{
        if (s1->nodes[i] != NULL)
		{
            if (__set_contains(s2, s1->nodes[i]->_key, s1->nodes[i]->_hash) == SET_TRUE)
			{
                __set_add(res, s1->nodes[i]->_key, s1->nodes[i]->_hash);
            }
        }
    }
    return SET_TRUE;
}

/* difference is s1 - s2 */
int set_difference(PointSet *res, PointSet *s1, PointSet *s2)
{
    if (res->used_nodes != 0)
	{
        return SET_OCCUPIED_ERROR;
    }
    // loop over s1 and keep only things not in s2
    for (uint64_t i = 0; i < s1->number_nodes; ++i)
	{
        if (s1->nodes[i] != NULL)
		{
            if (__set_contains(s2, s1->nodes[i]->_key, s1->nodes[i]->_hash) != SET_TRUE)
			{
                __set_add(res, s1->nodes[i]->_key, s1->nodes[i]->_hash);
            }
        }
    }
    return SET_TRUE;
}

int set_symmetric_difference(PointSet *res, PointSet *s1, PointSet *s2) 
{
    if (res->used_nodes != 0) return SET_OCCUPIED_ERROR;
    // loop over set 1 and add elements that are unique to set 1
    for (uint64_t i = 0; i < s1->number_nodes; ++i)
	{
        if (s1->nodes[i] != NULL)
		{
            if (__set_contains(s2, s1->nodes[i]->_key, s1->nodes[i]->_hash) != SET_TRUE)
			{
                __set_add(res, s1->nodes[i]->_key, s1->nodes[i]->_hash);
            }
        }
    }
    // loop over set 2 and add elements that are unique to set 2
    for (uint64_t i = 0; i < s2->number_nodes; ++i)
	{
        if (s2->nodes[i] != NULL)
		{
            if (__set_contains(s1, s2->nodes[i]->_key, s2->nodes[i]->_hash) != SET_TRUE)
			{
                __set_add(res, s2->nodes[i]->_key, s2->nodes[i]->_hash);
            }
        }
    }
    return SET_TRUE;
}

int set_is_subset(PointSet *test, PointSet *against)
{
    for (uint64_t i = 0; i < test->number_nodes; ++i)
	{
        if (test->nodes[i] != NULL)
		{
            if (__set_contains(against, test->nodes[i]->_key, test->nodes[i]->_hash) == SET_FALSE)
			{
                return SET_FALSE;
            }
        }
    }
    return SET_TRUE;
}

int set_is_subset_strict(PointSet *test, PointSet *against)
{
    if (test->used_nodes >= against->used_nodes) return SET_FALSE;
    return set_is_subset(test, against);
}

int set_cmp(PointSet *left, PointSet *right)
{
    if (left->used_nodes < right->used_nodes)
	{
        return SET_RIGHT_GREATER;
    }
	else if (right->used_nodes < left->used_nodes)
	{
        return SET_LEFT_GREATER;
    }
    for (uint64_t i = 0; i < left->number_nodes; ++i)
	{
        if (left->nodes[i] != NULL)
		{
            if (set_contains(right, left->nodes[i]->_key) != SET_TRUE)
			{
                return SET_UNEQUAL;
            }
        }
    }
    return SET_EQUAL;
}

// FIXME
int set_to_str(PointSet *set, char result[])
{
    for (uint64_t i = 0; i < set->number_nodes; ++i)
	{
		if (set->nodes[i] != NULL)
		{
			char *node_str = pt_to_str(*(set->nodes[i]->_key));
			strcat(result, node_str);
			strcat(result, "\n");
			if (node_str) free(node_str);
			else
			{
				fprintf(stderr, "%s() error: could not convert pt to string.\n", __func__);
				return SET_FALSE;	
			}	

		}
	}
	return SET_TRUE;
}

int print_set(PointSet *set)
{
	for (uint64_t i = 0; i < set->number_nodes; ++i)
	{
		if (set->nodes[i] != NULL)
		{
			print_pt(*(set->nodes[i]->_key));
		}
	}
	return SET_TRUE;
}

int chain_to_set(Point3D chain[], int N, PointSet *set)
{
	assert(N > 0);
	for (int i = 0; i < N; i++)
	{
		if (set_add(set, &(chain[i])) == SET_FALSE)
		{
			fprintf(stderr, "%s() error: could not convert chain to set.\n", __func__);
			return SET_FALSE;
		}
	}
	return SET_TRUE;
}

Point3D set_rand_choice(PointSet *set, threefry2x32_ctr_t *ctr, threefry2x32_key_t *key)
{
	// not sure if we can, uh, do this...
	float probs[set->used_nodes];
	// for now, choose an element with uniform probability
	// TODO: add support for generic pdf
	for (int i = 0; i < set->used_nodes; i++)
	{
		probs[i] = 1.0 / set->used_nodes;	
	}
	float cum_prob[set->used_nodes];
	float accum = 0.0;
	for (int i = 0; i < set->used_nodes; i++)
	{
		accum += probs[i];
		cum_prob[i] = accum;
	}
	assert(flt_near_eq(accum, 1.0, EPS));

	float rand_val = rand_flt(ctr, key, 0.0, 1.0);
	Point3D return_val;
	pt_init(&return_val);
	int prob_ctr = 0; /* need sep counter as we loop over all nodes, not just used */
    for (uint64_t i = 0; i < set->number_nodes; ++i)
	{
		if (set->nodes[i] != NULL)
		{
			if (flt_less_than(rand_val, cum_prob[prob_ctr], EPS))
			{
				pt_copy(set->nodes[i]->_key, &return_val);
				break;
			}
			prob_ctr++;
		}
	}	
	return return_val;
}

//char** set_to_array(PointSet *set, uint64_t *size) {
//    *size = set->used_nodes;
//    char** results = (char**)calloc(set->used_nodes + 1, sizeof(char*));
//    uint64_t i, j = 0;
//    size_t len;
//    for (i = 0; i < set->number_nodes; ++i) {
//        if (set->nodes[i] != NULL) {
//            len = strlen(set->nodes[i]->_key);
//            results[j] = (char*)calloc(len + 1, sizeof(char));
//            memcpy(results[j], set->nodes[i]->_key, len);
//            ++j;
//        }
//    }
//    return results;
//}

/*******************************************************************************
        					    PRIVATE FUNCTIONS
*******************************************************************************/
/*
   NOTE: this function looks a little bit clunky now because it now supports
  		 keys of type Point3D (struct point3d). So we have to iterate over
  		 every byte in each of the three float values which constitute the
  		 Point3D type.
 */
static uint64_t __default_hash(const Point3D *key)
{
    // FNV-1a hash (http://www.isthe.com/chongo/tech/comp/fnv/)
	unsigned char key_x_in_bytes[4];
	unsigned char key_y_in_bytes[4];
	unsigned char key_z_in_bytes[4];
	flt_to_bytes(key_x_in_bytes, key->x);
	flt_to_bytes(key_y_in_bytes, key->y);
	flt_to_bytes(key_z_in_bytes, key->z);

    uint64_t h = 14695981039346656037ULL; // FNV_OFFSET 64 bit
    for (size_t i = 0; i < 4; ++i)
	{
        h = h ^ key_x_in_bytes[i];
        h = h * 1099511628211ULL; // FNV_PRIME 64 bit
    }
    for (size_t i = 0; i < 4; ++i)
	{
        h = h ^ key_y_in_bytes[i];
        h = h * 1099511628211ULL; // FNV_PRIME 64 bit
    }
    for (size_t i = 0; i < 4; ++i)
	{
        h = h ^ key_z_in_bytes[i];
        h = h * 1099511628211ULL; // FNV_PRIME 64 bit
    }
    return h;
}

static int __set_contains(PointSet *set, const Point3D *key, uint64_t hash)
{
    uint64_t index;
    return __get_index(set, key, hash, &index);
}

static int __set_add(PointSet *set, const Point3D *key, uint64_t hash)
{
    uint64_t index;
    if (__set_contains(set, key, hash) == SET_TRUE) return SET_ALREADY_PRESENT;

    // Expand nodes if we are close to our desired fullness
    if ((float)set->used_nodes / set->number_nodes > MAX_FULLNESS_PERCENT)
	{
		uint64_t num_els = set->number_nodes * 2; // we want to double each time
		point_set_node** tmp = (point_set_node**)realloc(set->nodes, num_els * sizeof(point_set_node*));
		// malloc failure 
		if (tmp == NULL || set->nodes == NULL) return SET_MALLOC_ERROR;
		set->nodes = tmp;
		uint64_t i, orig_num_els = set->number_nodes;
		for (i = orig_num_els; i < num_els; ++i) set->nodes[i] = NULL;
		set->number_nodes = num_els;
		// re-layout all nodes
		__relayout_nodes(set, 0, 1);
    }
    // add element in
    int res = __get_index(set, key, hash, &index);
    if (res == SET_FALSE) // this is the first open slot
	{
        __assign_node(set, key, hash, index);
        ++set->used_nodes;
        return SET_TRUE;
    }
	// only other return value options are SET_TRUE or SET_CIRCULAR_ERROR 
	// so will most of time give same return code if 
	// we weren't able to add in the element (because we WERE able to return
	// the index) as if we were able to add in the element. Much lower relative
	// chance of returning SET_CIRCULAR_ERROR. However, how likely do we hit this
	// point?
    return res;
}

// TODO: test this function as well, as maybe the byte array representations
// 		 of the floats within a Point3D struct don't align perfectly...
// 		 ALSO is this implementation better/faster than just using our higher-lvl
// 		 pt_equal function??
static int __get_index(PointSet *set, const Point3D *key, uint64_t hash, uint64_t *index)
{
    uint64_t i, idx;
    idx = hash % set->number_nodes;
    i = idx;
	// typecast the input key
	const char *key_x_in_bytes = (const char *)&(key->x);
	const char *key_y_in_bytes = (const char *)&(key->y);
	const char *key_z_in_bytes = (const char *)&(key->z);
    size_t len_x = strlen(key_x_in_bytes);
    size_t len_y = strlen(key_y_in_bytes);
    size_t len_z = strlen(key_z_in_bytes);
    while (1)
	{
        if (set->nodes[i] == NULL)
		{
            *index = i;
            return SET_FALSE; // not here OR first open slot
        }
		// need to do a similar typecast for the key we're comparing against
		const char *_node_key_x = (const char *)&(set->nodes[i]->_key->x);
		const char *_node_key_y = (const char *)&(set->nodes[i]->_key->y);
		const char *_node_key_z = (const char *)&(set->nodes[i]->_key->z);
		if (hash == set->nodes[i]->_hash
			&& len_x == strlen(_node_key_x)
			&& len_y == strlen(_node_key_y)
			&& len_z == strlen(_node_key_z)
			&& (strncmp(key_x_in_bytes, _node_key_x, len_x) == 0)
			&& (strncmp(key_y_in_bytes, _node_key_y, len_z) == 0)
			&& (strncmp(key_z_in_bytes, _node_key_z, len_z) == 0))
		{
            *index = i;
            return SET_TRUE;
        }
        ++i;
        if (i == set->number_nodes) i = 0;
		// this means we went all the way around and the set is full
        if (i == idx) return SET_CIRCULAR_ERROR;
    }
}

// TODO: test this one out specifically, this was the trickiest alteration
static int __assign_node(PointSet *set, const Point3D *key, uint64_t hash, uint64_t index)
{
    set->nodes[index] = (point_set_node *)malloc(sizeof(point_set_node));
    set->nodes[index]->_key = (Point3D *)calloc(1, sizeof(Point3D));
    memcpy(set->nodes[index]->_key, key, sizeof(Point3D));
    set->nodes[index]->_hash = hash;
    return SET_TRUE;
}

// NOTE: key now is _key, which is NOT a pointer: do we still want to dealloc
// 		 it's location in memory?? (I think so, since __assign_node callocs)
static void __free_index(PointSet *set, uint64_t index)
{
    free(&(set->nodes[index]->_key));
    free(set->nodes[index]);
    set->nodes[index] = NULL;
}

static void __relayout_nodes(PointSet *set, uint64_t start, short end_on_null)
{
    uint64_t index = 0;
    for (uint64_t i = start; i < set->number_nodes; ++i)
	{
        if(set->nodes[i] != NULL)
		{
            __get_index(set, set->nodes[i]->_key, set->nodes[i]->_hash, &index);
            if (i != index) // we are moving this node
			{ 
                __assign_node(set, set->nodes[i]->_key, set->nodes[i]->_hash, index);
                __free_index(set, i);
            }
        }
		else if (end_on_null == 0 && i != start) break;
    }
}

