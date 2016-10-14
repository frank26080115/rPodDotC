#ifndef _OPTONCDT_H_
#define _OPTONCDT_H_

#include <stdint.h>

#include "uart.h"

#define OPTONCDT_HIGH_ERROR_VALUE 51
#define OPTONCDT_LOW_ERROR_VALUE -1

typedef enum
{
	OPTONCDT_FLAG_NONE = 0,
	OPTONCDT_FLAG_NEW = 1,
	OPTONCDT_FLAG_ERROR = 2,
}
optoncdt_flag_t;

typedef struct
{
	int id;
	uart_t* uart;
	uint8_t buff[3];
	float dist;
	uint32_t raw;
	uint32_t good;
	float* offset;
	int bad_cnt;
	optoncdt_flag_t flag;
}
laser_t;

void optoncdt_laserPowerFull(laser_t* laser);
void optoncdt_outputRs422(laser_t* laser);
void optoncdt_poll(laser_t* laser);
void optoncdt_structInit(laser_t* laser);

#endif