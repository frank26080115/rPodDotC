#ifndef _COMM_DEFS_H_
#define _COMM_DEFS_H_

#include <stdint.h>

enum
{
	OPCODE_PING, // receiver must reply with the same data buffer, but with "OPCODE_PONG" as the opcode
	OPCODE_PONG,
	OPCODE_BATT_TEMP,
	OPCODE_BATT_VOLTAGE,
	OPCODE_EMERGENCY_SHUTDOWN,
	OPCODE_WRITE_PARAM,
	OPCODE_WRITE_COMMIT,
};

// generic read and writes to arbitary registers
// convenient but adds a lot of overhead
typedef struct
{
	uint32_t addr;
	enum 
	{
		T_U64,
		T_S64,
		T_U32,
		T_S32,
		T_U16,
		T_S16,
		T_U8,
		T_S8,
		T_FLT,
		T_DBL,
	} typecode;
	union
	{
		uint64_t u64;
		int64_t s64;
		uint32_t u32;
		int32_t s32;
		uint16_t u16;
		int16_t s16;
		uint16_t u8;
		int16_t s8;
		float flt;
		double dbl;
	} data;
}
comm_setparam_t;

#endif