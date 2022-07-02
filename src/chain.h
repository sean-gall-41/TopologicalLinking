#ifndef CHAIN_H_
#define CHAIN_H_

#include <stdbool.h>
#include <limits.h>
#include "numerics.h"
#include "set.h"
#include "point3d.h"


#define MAX_CHAIN_LEN 200
#define MAX_CHAIN_STR_LEN 22400 // MAX_CHAIN_LEN * MAX_PT_STR_LEN 
#define EPS 1e-6f


void generate_random_chain(Point3D chain[], int N, float range_half_len,
	bool restrict_lattice, threefry2x32_ctr_t *ctr, threefry2x32_key_t *key);


void generate_chain_worm(Point3D chain[], int N, Point3D dirs[], int dirs_len,
	int dim, threefry2x32_ctr_t *ctr, threefry2x32_key_t *key);


void generate_closed_chain(Point3D chain[], int N, Point3D dirs[], int dirs_len,
	int dim, threefry2x32_ctr_t *ctr, threefry2x32_key_t *key);


void special_prob_dist(float probs[], int num_left, Point3D *node,
	int dim, Point3D dirs[], int num_dirs);


Point3D chain_rand_choice(Point3D dirs[], int num_dirs, float *probs,
	threefry2x32_ctr_t *ctr, threefry2x32_key_t *key);


void add_to_all(Point3D *to_add, Point3D add_to[],
	int add_to_len, Point3D result[]);


void chain_init(Point3D chain[], int N);


void chain_reset(Point3D chain[], int N);


bool is_reset(Point3D chain[], int N);


void chain_copy(Point3D chain[], int N, Point3D chain_cp[]);


bool chain_equal(Point3D chain_1[], int chain_1_len,
	Point3D chain_2[], int chain_2_len, float eps);


bool is_subset(Point3D compare[], int compare_len,
	Point3D compare_to[], int compare_to_len);


bool is_unique(Point3D chain[], int N, int less_than);


bool is_closed(Point3D chain[], int N);


bool is_sorted(Point3D chain[], int N);


void gen_all_bin_list3(Point3D bin_list[], int bin_list_len);


void chain_to_str(Point3D chain[], int chain_len, char result[]);


void print_chain(Point3D chain[], int chain_len);


void sort(Point3D chain[], int N);


void quicksort(Point3D chain[], int start, int end);


int partition(Point3D chain[], int start, int end);


int binary_search(Point3D *node_key, Point3D chain[], int N);


int binary_search_helper(Point3D *node_key, Point3D chain[],
	int p_0, int p_1);	


#endif /* CHAIN_H_ */
