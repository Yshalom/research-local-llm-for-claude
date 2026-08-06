#include "utils.h"

int32_t change_endian(int32_t v)
{
	return ((v << 24)	& 0xff000000)	// byte-1
		| ((v << 8)		& 0x00ff0000)	// byte-2
		| ((v >> 8)		& 0x0000ff00)	// byte-3
		| ((v >> 24)	& 0x000000ff);	// byte-4
}
