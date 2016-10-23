#ifndef _COMM_H_
#define _COMM_H_

#include <stdint.h>
#include "systmr.h"

#define COMM_MAGIC_1 0x5A
#define COMM_MAGIC_2 0x22 // these two must never be the same

typedef comm_sz_t uint8_t; // can be changed to make max packet size bigger but will have a bad impact on reliability and time to resync

typedef struct
{
	int id; // almost useless right now

	// HAL functions must be attached by user
	void (*hal_tx)(uint8_t); // pass in byte to send
	int16_t (*hal_rx)(); // returns -1 when no chars avail in buffer, otherwise return char
	// callback function must be attached by user
	void (*app_callback)(void*); // pass in comm_t* but cast to void*

	enum {
		COMMSTATE_WAITING,
		COMMSTATE_MAGIC_2,
		COMMSTATE_READING,
	} state;
	systmr_t tmr; // used for timeout
	uint32_t remainder; // tracks how many bytes left to read once the length field is available
	uint8_t reattempts; // since the sequence number is there, we can retransmit the same packet without confusing the receiver

	uint8_t buffer[260]; // stores data
	uint32_t buff_idx;

	uint8_t seq_rx; // previously received sequence number, so we can reject any duplicates
	uint8_t seq_tx; // previously sent sequence number, so we do not send the same one again
}
comm_t;

typedef struct
{
	uint8_t magic_1; // must always be COMM_MAGIC_1
	uint8_t magic_2; // must always be COMM_MAGIC_2
	comm_sz_t length; // length of the data segment
	uint8_t seq; // must increment or otherwise be different from previous, this messes with the checksum so that we can reject a stale buffer bug or bad DMA
	uint8_t opcode; // defined elsewhere
}
comm_pktheader_t;

// variable length data segment follows the header, and finally followed by a 16 bit checksum (fletcher16 algorithm)

#define COMM_LENGTH_IDX ((2*sizeof(uint8_t)) + sizeof(comm_sz_t))
#define COMM_DATA_START_IDX sizeof(comm_pktheader_t)

// public functions

// preinitialize variables in comm_t but does not assign the func pointers or ID
void comm_init(comm_t*);

// run this as fast as you want, the more frequently it runs, the less latency to received packets
void comm_task(comm_t*);

// builds and sends an entire packet, must fit within length, does checksumming automatically and sequence number handling automatically
void comm_send(comm_t*, uint8_t opcode, uint8_t* data, comm_sz_t len);

// forces the receiving device to be in sync, does so by blasting it with zeros
void comm_resync(comm_t*);

#endif