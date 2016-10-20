#ifndef _ASI_H_
#define _ASI_H_

#include <stdint.h>

#define ASI_DATA_STORE_SIZE 8
#define ASI_ENGINE_COUNT 8

enum
{
	ASIFLAG_NONE = 0x00,
	ASIFLAG_SINGLE_NEW_DATA = 0x01,
	ASIFLAG_ALL_NEW_DATA = 0x02,
	ASIFLAG_ERROR = 0x04,
};

typedef struct
{
	int id;
	uint8_t dev_addr;
	uint16_t data[ASI_DATA_STORE_SIZE];
	char flags;
}
engine_data_t;

extern engine_data_t asi_engines[ASI_ENGINE_COUNT];

char asi_poll(void);
void asi_swInit(uint8_t start_addr, uint8_t inc);

#endif