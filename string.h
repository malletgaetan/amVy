#ifndef STRING_H
# define STRING_H

# include <unistd.h>
# include <stddef.h>
# include "types.h"

struct String {
	const char *str;
	size_t size;
};

void put_string(struct String string);

#endif
