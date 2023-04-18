#include <stdlib.h>

#include <uk/assert.h>

_Noreturn void abort(void)
{
	UK_CRASH("abort called\n");
}