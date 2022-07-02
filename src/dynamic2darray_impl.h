#include <algorithm>
#include <cassert>

const int DEFAULT_SIZE = 1;

template<typename type>
Dynamic2DArray<type>::Dynamic2DArray()
{
	num_rows     = DEFAULT_SIZE;
	num_cols     = DEFAULT_SIZE;
	
	array.resize(num_rows * num_cols);
}

template <typename type>
Dynamic2DArray<type>::Dynamic2DArray(int num_rows, int num_cols):
	num_rows(num_rows), num_cols(num_cols)
{
	array.resize(num_rows * num_cols);	
}

template <typename type>
Dynamic2DArray<type>::Dynamic2DArray(int num_rows, int num_cols, type value):
	num_rows(num_rows), num_cols(num_cols)
{
	array.resize(num_rows * num_cols);
	fill(value);
}

template <typename type>
Dynamic2DArray<type>::Dynamic2DArray(const Dynamic2DArray &copyFrom)
{
	Dynamic2DArray<type>(copyFrom.num_rows, copyFrom.num_cols);	
	std::copy(copyFrom.array.begin(), copyFrom.array.end(), this->array.begin());
}

template <typename type>
Dynamic2DArray<type>::~Dynamic2DArray() {}

template <typename type>
void Dynamic2DArray<type>::fill(type value)
{
	std::fill(array.begin(), array.end(), value);	
}

template <typename type>
int Dynamic2DArray<type>::size() const
{
	return array.size();
}

template <typename type>
int Dynamic2DArray<type>::maxSize() const
{
	return array.max_size();
}

template <typename type>
bool Dynamic2DArray<type>::empty() const
{
	return array.empty();
}

template <typename type>
void Dynamic2DArray<type>::reshape(int new_num_rows, int new_num_cols)
{
	array.resize(new_num_rows * new_num_cols);
	
	num_rows = new_num_rows;
	num_cols = new_num_cols;
}
template <typename type>
std::vector<type>& Dynamic2DArray::row_as_vector(int row_index)
{
	assert(row_index >= 0);
	assert(row_index < num_rows);
	std::vector<type> this_row(num_cols, 0);
	size_t start = row_index * num_cols;
	size_t end   = start + num_cols;
	for (size_t i = start; i < end; i++)
	{
		this_row[i] = array[i];
	}	
	return this_row;
}

template <typename type>
std::vector<type>& Dynamic2DArray::col_as_vector(int col_index)
{
	assert(colIndex >= 0);
	assert(colIndex < num_cols);
	std::vector<type> this_col(num_rows, 0);
	for (size_t i = 0; i < num_rows, i++)
	{
		this_col[i] = array[col_index + i * num_cols];
	}
	return this_col;
}

template <typename type>
type* Dynamic2DArray<type>::data() const
{
	return array.data();
}

template <typename type>
type &Dynamic2DArray<type>::operator()(int row, int col)
{
	return array[row * num_cols + col];
}	

template <typename t>
bool operator==(const Dynamic2DArray<t> &this_arr,
	const Dynamic2DArray<t> &other_arr)
{ 
	return (this_arr.num_rows == other_arr.num_rows)
		&& (this_arr.num_cols == other_arr.num_cols) 
		&& (this_arr.array == other_arr.array);
}

template <typename t>
bool operator!=(const Dynamic2DArray<t> &this_arr,
	const Dynamic2DArray<t> &other_arr) 
{
	return !(this_arr == other_arr);
}

template <typename type>
Dynamic2DArray<type> &Dynamic2DArray<type>::operator=(const Dynamic2DArray<type> &other_arr)
{
	if (this != &other_arr)
	{
		this->array   = other_arr.array;
		this->num_rows = other_arr.num_rows;
		this->num_cols = other_arr.num_cols;
	}

	return *this;
}

