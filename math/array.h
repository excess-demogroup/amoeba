/*
 * An automatically scaling array implementation -- no bounds checking.
 */

#ifndef _ARRAY_H
#define _ARRAY_H

#include <stdlib.h>

template <class T> class Array {
	typedef T * iterator;
	typedef const T * const_iterator;
	
protected:
	T *array;
	unsigned int room, elem;
	
public:
	Array()
	{
		this->array = (T *)(malloc(sizeof(T)));
		this->room = 1;
		this->elem = 0;
	}
	
	~Array()
	{
		free(this->array);
		this->array = NULL;
	}

	const Array<T> &operator=(Array<T> const &other)
	{
		if (this != &other) {
			free(this->array);
			this->room = this->elem = other.num_elems();
			this->array = (T *)malloc(this->room * sizeof(T));
			memcpy(this->array, other.get_array(), this->room * sizeof(T));
		}
		return *this;
	}
	
	inline T &operator[](const unsigned int index)
	{
		if (index >= room) {
			while (index >= room) {
				room <<= 1;
			}
			this->array = (T *)realloc(this->array, room * sizeof(T));
		}
		if (index >= elem) elem = index + 1;
		return this->array[index];
	}

	inline T operator[](const unsigned int index) const
	{
		return this->array[index];
	}

	/* duplicated code here, but I'm too lazy to fix it ;-) */
	inline void add_end(T t)
	{
		if (elem == room) {
			room <<= 1;
			this->array = (T *)realloc(this->array, room * sizeof(T));
		}
		this->array[this->elem++] = t;
	}

	inline int num_elems() const
	{
		return elem;
	}

	inline T *get_array() const
	{
		return this->array;
	}

	inline iterator begin()
	{
		return array;
	}

	inline const_iterator begin() const
	{
		return array;
	}
	
	inline iterator end()
	{
		return array + elem;
	}
	
	inline const_iterator end() const
	{
		return array + elem;
	}
};

#endif /* defined(_ARRAY_H) */
