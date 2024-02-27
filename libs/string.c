#include "libs/string.h"

int put_string(struct String string)
{
	return write(STDOUT_FILENO, string.str, string.size);
}

