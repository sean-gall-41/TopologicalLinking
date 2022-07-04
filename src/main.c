//#include "chain.h"

#include "gui.h"

#define NUM_DIRS 8
#define DIM 3
#define CHAIN_LEN 150 


int main(int argc, char *argv[]) 
{
	//// initialize the random seed
	//threefry2x32_ctr_t ctr = {{0, 0}};
	//threefry2x32_key_t key = {{0, 0}};

	//// generate all dirs on cubic lattice
	//Point3D dirs[NUM_DIRS];
	//gen_all_bin_list3(dirs, NUM_DIRS);

	//// initialize the chain
	//Point3D chain[CHAIN_LEN];
	//chain_init(chain, CHAIN_LEN);

	//// generate the SAW on the cubic lattice
	//generate_closed_chain(chain, CHAIN_LEN, dirs, NUM_DIRS, DIM, &ctr, &key);
	//print_chain(chain, CHAIN_LEN);
	return (gui_init(&argc, &argv) && gui_run()) ? 0 : 1;
}

