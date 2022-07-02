#include <stdio.h>
#include <float.h>
#include "chain.h"


void generate_random_chain(Point3D chain[], int N, float range_half_len,
	bool restrict_lattice, threefry2x32_ctr_t *ctr, threefry2x32_key_t *key)
{
	assert(N > 0);
	if (restrict_lattice && (range_half_len < 1.0))
	{
		fprintf(stderr, "requested rand range to lie on axis, but upper bound is > 1.0.");
		exit(1);
	}

	for (int i = 0; i < N; i++)
	{
		if (restrict_lattice)
		{
			chain[i].x = (float)rand_int(&ctr, &key, 0, (int)range_half_len); 	
			chain[i].y = (float)rand_int(&ctr, &key, 0, (int)range_half_len); 	
			chain[i].z = (float)rand_int(&ctr, &key, 0, (int)range_half_len); 	
		}
		else
		{
			chain[i].x = rand_flt(&ctr, &key, 0.0, range_half_len); 	
			chain[i].y = rand_flt(&ctr, &key, 0.0, range_half_len); 	
			chain[i].z = rand_flt(&ctr, &key, 0.0, range_half_len); 	
		}
	}
}


void generate_chain_worm(Point3D chain[], int N, Point3D dirs[], int dirs_len,
	int dim, threefry2x32_ctr_t *ctr, threefry2x32_key_t *key)
{
	Point3D node;
	Point3D dir;
	pt_init(&node);
	pt_init(&dir);
		
	// loop over all possible nodes (first is at origin)
	for (int i = 1; i < N; i++)
	{
		// first, check if we are locked out
		// create surrounding array
		Point3D surrounding[dirs_len];
		chain_init(surrounding, dirs_len);		
		add_to_all(&node, dirs, dirs_len, surrounding);

		// create set versions of relevant arrays
		PointSet chain_as_set, surrounding_as_set, remaining_surrounding;
		set_init(&chain_as_set);
		set_init(&surrounding_as_set);
		set_init(&remaining_surrounding);
		chain_to_set(chain, N, &chain_as_set);
		chain_to_set(surrounding, dirs_len, &surrounding_as_set);
		// check to see if we are locked out
		if (set_is_subset(&surrounding_as_set, &chain_as_set) == SET_TRUE)
		{
			chain_reset(chain, N);
		 	return; 
		}

		// if not locked out, choose a neighbor from special pdf
		float probs[dirs_len];
		special_prob_dist(probs, N - i, &node, dim, dirs, dirs_len);	
		dir = chain_rand_choice(dirs, dirs_len, probs, ctr, key);	
		Point3D new_node = pt_add(&node, &dir);

		// while we choose an already occupied node, find an unoccupied neighbor
		if (set_contains(&chain_as_set, &new_node) == SET_TRUE)
		{
			// find the set of all unoccupied neighbors
			set_difference(&remaining_surrounding, &surrounding_as_set, &chain_as_set);	
			new_node = set_rand_choice(&remaining_surrounding, ctr, key);	
		}
		// once we get a unique new node, add it to the chain and set as
		// current node for next iteration	
		pt_copy(&new_node, &chain[i]);
		pt_copy(&new_node, &node);
	}
}


void generate_closed_chain(Point3D chain[], int N, Point3D dirs[], int dirs_len,
	int dim, threefry2x32_ctr_t *ctr, threefry2x32_key_t *key)
{
	assert(N > 0);
	assert(dirs_len > 0); 
	int attempts = 0;
	// case work: 1) initialized chain, all zeros,
	// or 2) we had to give up because we were locked out,
	// or 3) we generate a chain, but it's not closed
	while (is_reset(chain, N) || !is_closed(chain, N))
	{
		generate_chain_worm(chain, N, dirs, dirs_len, dim, ctr, key);	
		attempts++;
	}
	printf("Took %d attempts to generate (%d)-SAW.\n", attempts, N);
}


void special_prob_dist(float probs[], int num_left, Point3D *node,
	int dim, Point3D dirs[], int num_dirs)
{
	float sum = 0.0;
	for (int i = 0; i < num_dirs; i++)
	{
		float prob = 1.0;
		prob *= (num_left - (dirs[i].x * node->x)) / (2 * num_left);	
		prob *= (num_left - (dirs[i].y * node->y)) / (2 * num_left);	
		prob *= (num_left - (dirs[i].z * node->z)) / (2 * num_left);	
		if (flt_less_than(prob, 0.0, EPS)) prob = 0.0;
		probs[i] = prob;
		sum += prob;
	}
	
	for (int i = 0; i < num_dirs; i++)
	{
		probs[i] /= sum;
	}	
}


Point3D chain_rand_choice(Point3D dirs[], int num_dirs, float *probs,
	threefry2x32_ctr_t *ctr, threefry2x32_key_t *key)
{
	float cum_prob[num_dirs];
	float accum = 0.0;
	for (int i = 0; i < num_dirs; i++)
	{
		accum += probs[i];
		cum_prob[i] = accum;
	}
	assert(flt_near_eq(accum, 1.0, EPS));

	float rand_val = rand_flt(ctr, key, 0.0, 1.0);
	Point3D return_val;
	pt_init(&return_val);
	for (int i = 0; i < num_dirs; i++)
	{
		if (flt_less_than(rand_val, cum_prob[i], EPS))
		{
			pt_copy(&(dirs[i]), &return_val);
			break;
		}
	}	
	return return_val;
}


void add_to_all(Point3D *to_add, Point3D add_to[],
	int add_to_len, Point3D result[])
{
	assert(add_to_len > 0);
	for (int i = 0; i < add_to_len; i++)
	{
		result[i] = pt_add(to_add, &add_to[i]);
	}
}


void chain_init(Point3D chain[], int N)
{
	assert(N > 0);
	for (int i = 0; i < N; i++)
	{
		pt_init(&chain[i]);
	}
}


void chain_reset(Point3D chain[], int N)
{
	for (int i = 0; i < N; i++)
	{
		pt_reset(&chain[i]);
	}
}


bool is_reset(Point3D chain[], int N)
{
	assert(N > 0);
	Point3D zeros;
	pt_init(&zeros);
	bool result = true;
	for (int i = 0; i < N; i++)
	{
		result = result && pt_equal(&chain[i], &zeros, EPS); 
		if (!result) return false;
	}
	return true;
}


void chain_copy(Point3D chain[], int N, Point3D chain_cp[])
{
	for (int i = 0; i < N; i++)
	{
		// Potentially DANGEROUS 
		pt_copy(&chain[i], &chain_cp[i]);
	}
}


bool chain_equal(Point3D chain_1[], int chain_1_len,
	Point3D chain_2[], int chain_2_len, float eps)
{
	if (chain_1_len != chain_2_len) return false;
	for (int i = 0; i < chain_1_len; i++)
	{
		if (!pt_equal(&chain_1[i], &chain_2[i], eps)) false;
	}
	return true;
}

bool is_subset(Point3D compare[], int compare_len,
	Point3D compare_to[], int compare_to_len)
{
	// assume compare and compare_to are both unique
	if (compare_len > compare_to_len) return false;
	// define set itself as its own subset
	if (chain_equal(compare, compare_len,
		compare_to, compare_to_len, EPS)) return true;

	for (int i = 0; i < compare_len; i++)
	{
		// FIXME: what does the binary search algorithm assume our input will look like?
		// 		  what if we give it the zero array?
		int key_index = binary_search(&compare[i], compare_to, compare_to_len);
		if (key_index == -1) return false;
	}
	return true;
}

/*
 * less_than indicates that we wish to say the chain
 * is unique **up to** element at index less_than.
 * This is done because
 */
bool is_unique(Point3D chain[], int N, int less_than)
{
	assert(less_than > 0);
	assert(less_than <= N);
	// only index less than 1 is 0, so must be unique
	if (less_than == 1) return true;
	Point3D seen[less_than];
	for (int i = 0; i < less_than; i++)
	{
		// need better initialization values
		seen[i].x = FLT_MAX;
		seen[i].y = FLT_MAX;
		seen[i].z = FLT_MAX;
	}
	// loop through points beyond the starting index
	// and check whether we have seen this point before
	for (int i = 0; i < less_than; i++)
	{
		printf("testing...\n");
		if (binary_search(&chain[i], seen, less_than) != -1)
		{
			return false;
		}
		pt_copy(&chain[i], &seen[i]);
	}
	return true;
}


bool is_closed(Point3D chain[], int N)
{
	assert(N > 0);
	if (is_reset(chain, N)) return false;
	Point3D difference = pt_subtr(&chain[0], &chain[N - 1]);
	return (flt_near_eq(pt_norm_sq(&difference), 3.0, EPS)) ? true : false;
}


/*
 * including the case of equality, as we want, say, the zero chain to be sorted.
 * (I think)
 */
bool is_sorted(Point3D chain[], int N)
{
	if (is_reset(chain, N)) return true;	
	for (int i = 0; i < N - 1; i++)
	{
		if (pt_great_than(&chain[i], &chain[i + 1], EPS)) return false;
	}	
	return true;
}


void gen_all_bin_list3(Point3D bin_list[], int bin_list_len)
{
	assert(bin_list_len > 0);
	int dir_index = 0;
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			for (int k = 0; k < 2; k++)
			{
				bin_list[dir_index].x = 2 * i - 1;
				bin_list[dir_index].y = 2 * j - 1;
				bin_list[dir_index].z = 2 * k - 1;
				dir_index++;
			}
		}
	}
}


void chain_to_str(Point3D chain[], int chain_len, char result[])
{
	for (int i = 0; i < chain_len; i++)
	{
		char *node_str = pt_to_str(chain[i]);
		strcat(result, node_str);
		strcat(result, "\n");
		if (node_str) free(node_str);
	}
}


void print_chain(Point3D chain[], int chain_len)
{
	assert(chain_len > 0);
	for (int i = 0; i < chain_len; i++)
	{
		print_pt(chain[i]);
	}
}


void sort(Point3D chain[], int N)
{
	quicksort(chain, 0, N - 1);
}


void quicksort(Point3D chain[], int start, int end)
{
	if (start >= end) return;
	int boundary = partition(chain, start, end);
	quicksort(chain, start, boundary - 1);
	quicksort(chain, boundary + 1, end);
}


int partition(Point3D chain[], int start, int end)
{
	Point3D pivot;
	pt_copy(&chain[start], &pivot);
	int lh = start + 1;
	int rh = end;
	while (true) 
	{
		while (lh < rh && pt_great_than_or_eq(&chain[rh], &pivot, EPS)) rh--;
		while (lh < rh && pt_less_than(&chain[lh], &pivot, EPS)) lh++;
		if (lh == rh) break;
		Point3D temp;
		pt_copy(&chain[lh], &temp);
		pt_copy(&chain[rh], &chain[lh]);
		pt_copy(&temp, &chain[rh]);
	}
	if (pt_great_than_or_eq(&chain[lh], &pivot, EPS)) return start;
	pt_copy(&chain[lh], &chain[start]);
	pt_copy(&pivot, &chain[lh]);
	return lh;
}


int binary_search(Point3D *node_key, Point3D chain[], int N) 
{
	if (!is_sorted(chain, N))
	{
		Point3D chain_cp[N];
		chain_copy(chain, N, chain_cp);
		sort(chain_cp, N);
		return binary_search_helper(node_key, chain_cp, 0, N - 1);	
	}
	return binary_search_helper(node_key, chain, 0, N - 1);	
}

//FIXME: TRIPLE TEST THE FLT COMPARISONS!
int binary_search_helper(Point3D *node_key, Point3D chain[],
	int p_0, int p_1)	
{
	if (p_0 > p_1) return -1;
	int mid = (p_0 + p_1) / 2;
	if (pt_equal(node_key, &chain[mid], EPS)) return mid;
	if (pt_less_than(node_key, &chain[mid], EPS))
	{
		return binary_search_helper(node_key, chain, p_0, mid - 1);
	} 
	else
	{
		return binary_search_helper(node_key, chain, mid + 1, p_1);
	}
}

