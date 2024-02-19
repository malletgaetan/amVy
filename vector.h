#ifndef VECTOR_H
# define VECTOR_H

# include <assert.h>
# include <stdlib.h>

typedef struct {
	size_t len;
	size_t capacity;
} VectorMetadata;

# define vector_add(arr, element)																																	\
{																																									\
	if ((arr) == NULL)																																				\
	{																																								\
		(arr) = malloc((sizeof((element)) * 2) + sizeof(VectorMetadata));																							\
		assert(arr);																																				\
		((VectorMetadata *)(arr))[0].capacity = 2;																													\
		((VectorMetadata *)(arr))[0].len = 1;																														\
		(arr) = (typeof(element) *)(((VectorMetadata *)(arr)) + 1);																									\
		((typeof(element) *)(arr))[0] = (element);																													\
	}																																								\
	else																																							\
	{																																								\
		if (((VectorMetadata *)(arr))[-1].capacity <= ((VectorMetadata *)(arr))[-1].len)																			\
		{																																							\
			((VectorMetadata *)(arr))[-1].capacity *= 2;																											\
			(arr) = realloc((void *)(((VectorMetadata *)(arr)) - 1), (size_t)(((VectorMetadata *)(arr))[-1].capacity * sizeof(element)) + sizeof(VectorMetadata));	\
			assert(arr);																																			\
			(arr) = (typeof(element) *)((VectorMetadata *)(arr) + 1);																								\
		}																																							\
		((typeof(element) *)(arr))[(((VectorMetadata *)(arr))[-1].len++)] = element;																				\
	}																																								\
}

# define vector_destroy(arr) free(((VectorMetadata *)(arr)) - 1)

# define vector_size(arr) ((VectorMetadata *)(arr))[-1].len

#endif
