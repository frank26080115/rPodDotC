/*
handles communications over UART
designed to replace rI2CTX and rI2CRX
*/

#include "comm.h"
#include "systmr.h"
#include "fletcher.h"

void comm_init(comm_t comm)
{
	com->state = COMMSTATE_WAITING; // wait for sync
	com->tmr = systmr_nowMillis();
	com->seq_rx = 0xAA; // start high but non-zero, less chance of the first packet received being wrongly interpretted as a duplicate
	com->seq_tx = 0x05; // start low but non-zero, less chance of the first packet sent being wrongly interpretted as a duplicate
	com->reattempts = 0;
	// note: the function pointers and the ID are assigned in user application layer
}

void comm_task(comm_t com)
{
	int16_t c;

	comm_pktheader_t* hdr = (comm_pktheader_t*)com->buffer;

	c = com->hal_read();
	if (c < 0) // nothing in buffer
	{
		if ((systmr_nowMillis() - com->tmr) > 100) { // timeout
			com->state = COMMSTATE_WAITING;
			com->buff_idx = 0;
		}

		return;
	}

	if (com->state == COMMSTATE_WAITING || com->state == COMMSTATE_MAGIC_2)
	{
		if (c == COMM_MAGIC_1) // got sync word
		{
			com->buff_idx = 0; // so start for the first
			com->buffer[com->buff_idx++] = c;
			state = COMMSTATE_MAGIC_2; // we have the sync word
			tmr = systmr_nowMillis();
			return;
		}
		else if (com->state == COMMSTATE_MAGIC_2 && c == COMM_MAGIC_2)
		{
			com->buff_idx = 1;
			com->buffer[com->buff_idx++] = c;
			com->state = COMMSTATE_READING; // we have both sync words
			com->tmr = systmr_nowMillis();
			com->remainder = 0;
			return;
		}
		else
		{
			com->state = COMMSTATE_WAITING; // reset due to error, not in sync
		}
	}
	else
	{
		com->tmr = systmr_nowMillis(); // prevent timeout
		com->buffer[com->buff_idx++] = c; // save

		if (com->buff_idx == COMM_LENGTH_IDX) // length indicator just became available
		{
			com->remainder = hdr->length + 1 + 1 + 2; // add on opcode, seq, and checksum
		}
		else if (com->buff_idx > COMM_LENGTH_IDX) // length indicator is available
		{
			com->remainder -= 1;
			if (com->remainder <= 0) { // this is the final byte
				comm_handle(com); // process whole packet
				com->state = COMSTATE_WAITING; // we are done, try to sync again
				com->buff_idx = 0;
			} 
		}
		else {
			// do nothing
		}
	}
}

void comm_handle(comm_t* com)
{
	uint16_t checksum_calculated;
	uint16_t* checksum_received;

	comm_pkthdr_t* hdr = (comm_pkthdr_t*)com->buffer;
	if (hdr->seq == com->seq_rx) {
		// outdated packet
		// TODO: report error
		return;
	}

	checksum_calculated = fletcher16(com->buffer, com->buff_idx - 2);
	checksum_received = (uint16_t*)&(com->buffer[com->buff_idx - 2]);

	if (checksum_calculated != *checksum_received) {
		// bad packet
		// TODO: report error
		return;
	}

	app_handleComm(com); // callback to app

	com->seq_rx = hdr->seq; // remember what the previous sequence number was so we can reject a repeat
}

void comm_send(comm_t* com, uint8_t opcode, uint8_t* data, comm_sz_t len)
{
	int idx_inner, idx_outer, final_len;
	int reattempt;
	static uint8_t outbuff[COMM_MAX_TX_SIZE];
	comm_pktheader_t* hdr = (comm_pktheader_t*)outbuff;
	uint16_t checksum;

	// prepare header
	hdr->magic_1 = COMM_MAGIC_1;
	hdr->magic_2 = COMM_MAGIC_2;
	hdr->length = len;
	hdr->seq = com->seq_tx++;
	hdr->opcode = opcode;

	// copy inner data
	for (idx_inner = 0, idx_outer = sizeof(comm_pktheader_t); idx_inner < len && idx_outer < COMM_MAX_TX_SIZE - 2; idx_inner++, idx_outer++)
	{
		outbuff[idx_outer] = data[idx_inner];
	}

	// calculate and assign checksum, little endian
	checksum = fletcher16(outbuff, idx_outer);
	outbuff[idx_outer++] = (uint8_t)(checksum & 0xFF);
	outbuff[idx_outer++] = (uint8_t)(checksum >> 8);

	final_len = idx_outer;

	// minimum of 1 attempt must be re-enforced
	if (com->reattempts <= 0) {
		com->reattempts = 1;
	}

	// reattempts are harmless if the bandwidth is available, due to the way the sequence number works
	for (reattempt = 0; reattempt < com->reattempts; reattempt++)
	{
		// send all data
		for (idx_inner = 0; idx_inner < final_len; idx_inner++)
		{
			com->hal_tx(outbuff[i]);
		}
	}
}

// blasting the receiver with zeros will eventually force it into COMMSTATE_WAITING
void comm_resync(comm_t* com)
{
	int flood;
	for (flood = 0; flood < 256; flood++) {
		com->hal_tx(0);
	}
}