#include "optoncdt.h"

#define OPTONCDT_ERROR_VALUE 65467
#define OPTONCDT_MIDDLE_VALUE (OPTONCDT_ERROR_VALUE / 2)

#define OPTONCDT_ERROR_COUNT_LIMIT 100

void optoncdt_processSample(laser_t* laser);

void optoncdt_laserPowerFull(laser_t* laser)
{
	uint8_t buff[32];
	int mlen;
	mlen = sprintf((char*)buff, "LASERPOW FULL\r\n");
	laser->uart->writeBuff(buff, mlen);
}

void optoncdt_outputRs422(laser_t* laser)
{
	uint8_t buff[32];
	int mlen;
	mlen = sprintf((char*)buff, "OUTPUT RS422\r\n");
	laser->uart->writeBuff(buff, mlen);
}

void optoncdt_poll(laser_t* laser)
{
	while (laser->uart->avail() > 0)
	{
		uint8_t c = laser->uart->read();
		if ((c & (1 << 7)) != 0) // higher bits, sent last
		{
			laser->buff[2] = c;
			optoncdt_processSample(laser);
		}
		else if ((c & 0xC0) != 0) // middle bits
		{
			laser->buff[1] = c;
		}
		else if ((c & 0xC0) == 0) // lower bits
		{
			laser->buff[0] = c;
		}
	}
}

void optoncdt_processSample(laser_t* laser)
{
	// assemble data packet according to datasheet
	uint32_t result = buff[0] & 0x3F;
	uint32_t mid = buff[1] & 0x3F;
	uint32_t hi = buff[2] & 0x0F;
	result += mid << 6;
	result += hi << 12;

	laser->raw = result;
	if (result == OPTONCDT_ERROR_VALUE)
	{
		if (laser->bad_cnt > OPTONCDT_ERROR_COUNT_LIMIT)
		{
			if (laser->good > OPTONCDT_MIDDLE_VALUE) {
				laser->dist = OPTONCDT_HIGH_ERROR_VALUE;
			}
			else {
				laser->dist = OPTONCDT_LOW_ERROR_VALUE;
			}
			laser->flag = OPTONCDT_FLAG_ERROR;
		}
		else
		{
			laser->bad_cnt++;
		}
	}
	else
	{
		// convert to millimeters
		float y = raw;
		y *= 102.0;
		y /= 65520.0;
		y -= 1.0;
		y *= 50.0;
		y /= 100.0;
		laser->dist = y;

		laser->good = result;
		laser->bad_cnt = 0; // clear error
		laser->flag = OPTONCDT_FLAG_NEW;
	}
}

void optoncdt_structInit(laser_t* laser)
{
	laser->dist = 0;
	laser->flag = 0;
	laser->raw = 0;
	laser->bad_cnt = 0;
}