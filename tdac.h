#ifndef _TDAC_H_
#define _TDAC_H_

#include <stdint.h>

typedef struct
{
	uint8_t i2c_addr;
	uint16_t val[8];
}
tdac_t;

void tdac_init(tdac_t* dac);
void tdac_set(tdac_t* dac, uint8_t idx, uint16_t val);

#endif