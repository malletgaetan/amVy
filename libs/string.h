#ifndef STRING_H
# define STRING_H

# include <unistd.h>
# include <stddef.h>

struct String {
	const char *str;
	size_t size;
};

int put_string(struct String string);

#endif
