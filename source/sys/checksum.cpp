

#include "checksum.h"

//TO DO: units as circles collision

unsigned char AddCheck(unsigned char sum, unsigned char next)
{
	sum++;
	sum = sum ^ next;
	sum = (sum << 1) + (sum >> 7);
	return sum;
}