#include "string.h"

void put_string(struct String string)
{
	write(STDOUT_FILENO, string.str, string.size);
}

