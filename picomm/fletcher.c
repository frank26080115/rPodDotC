#include "fletcher.h"

// ripped verbatim from Wikipedia
// this is better that just summing or just XORing
// because this algorithm is position-dependant
// meaning when you checksum 01 02 03, it does not end up being the same checksum as 02 01 03

uint16_t fletcher16(uint8_t* data, int count)
{
	uint16_t sum1 = 0;
	uint16_t sum2 = 0;
	int index;

	for( index = 0; index < count; ++index )
	{
		sum1 = (sum1 + data[index]) % 255;
		sum2 = (sum2 + sum1) % 255;
	}

	return (sum2 << 8) | sum1;
}