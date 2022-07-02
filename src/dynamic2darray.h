#ifndef DYNAMIC2DARRAY_
#define DYNAMIC2DARRAY_ 

#include <vector>
#include <cstddef>

template <typename type>
class Dynamic2DArray
{
public:
	Dynamic2DArray();
	Dynamic2DArray(int num_row, int num_col);
	Dynamic2DArray(int num_row, int num_col, type value);
	Dynamic2DArray(const Dynamic2DArray &copyFrom);
	~Dynamic2DArray();

	void fill(type value);
	int size() const;
	int max_size() const;
	bool empty() const;
	void reshape(int new_num_rows, int new_num_cols);
	std::vector<type>& row_as_vector(int row_index);
	std::vector<type>& col_as_vector(int col_index);
	
	type* data() const;

	// much easier than overloading '[]'
	type &operator()(int row, int col);	

	// equality	
	template <typename t>
	friend bool operator==(const Dynamic2DArray<t> &this_arr,
		const Dynamic2DArray<t> &other_arr);
	template <typename t>
	friend bool operator!=(const Dynamic2DArray<t> &this_arr,
		const Dynamic2DArray<t> &other_arr);

	Dynamic2DArray<type> &operator=(const Dynamic2DArray<type> &other_arr);

private:
	int num_rows;
	int num_cols;
	
	// in future, revert back to array
	std::vector<type> array;
};

#include "dynamic2darray_impl.h"

#endif /* DYNAMIC2DARRAY_ */ 

