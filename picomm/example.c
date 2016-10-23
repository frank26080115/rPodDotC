#include "comm.h"
#include <stdint.h>

#define NUM_SENSORS 540
#define CHUNK_SIZE 8

// pretend these are HAL functions that somebody else already wrote, and they are in another file, hence "extern"
extern void ser_tx(uint8_t);
extern int16_t ser_tx(void);

void handle_comm(void*);

// notice how the function prototypes (sometimes AKA function signatures) matches the function pointers within the comm_t struct
// the functions themselves obviously need to be written with return types and parameters that match the prototype

comm_t rasp; // this represents a connection to a raspberry pi
// the reason why I chose to encapsulate it into a struct is so you can talk to multiple devices, each with its own serial port

void initialization()
{
	comm_init(&rasp);
	rasp.id = 1;
	rasp.hal_tx = ser_tx;
	rasp.hal_rx = ser_rx;
	rasp.app_callback = handle_comm;
	rasp.reattempts = 0;
}

void batt_temp_process()
{
	int16_t sensor_data[NUM_SENSORS];
	comm_setparam_t params[CHUNK_SIZE];

	// pretend we have sensor data
	// and we are sending it to the ground station

	// first of all, realize that 540 sensors is a fucktonne of data for one packet, so we send it in chunks
	// the longer the packets are, you lose more data when you do experience an error of some sort
	// but the shorter the packets are, the packet header adds some overhead, and that overhead accumulates faster, slowing down total speed

	for (int sensor_idx = 0, chunk_idx = 0; sensor_idx < NUM_SENSORS; sensor_idx++) {
		comm_setparam_t* p = &params[chunk_idx];
		p->addr = sensor_idx; // which sensor
		p->type = T_S16; // what is the data type
		p->data.s16 = sensor_data[sensor_idx]; // what is the sensor data
		chunk_idx++;
		if (chunk_idx >= CHUNK_SIZE || sensor_idx == (NUM_SENSORS - 1)) { // we filled a chunk or last chunk
			// send it all
			comm_send(&rasp, OPCODE_BATT_TEMP, (uint8_t*)params, chunk_idx * sizeof(comm_setparam_t));
			chunk_idx = 0; // new chunk
		}
	}
}

void handle_comm(void* ptr)
{
	comm_t* com = (comm_t*)ptr; // converts the void* back into a comm_t* so we can access the internals of our struct without messing with indices
	comm_pktheader_t* hdr = (comm_pktheader_t*)com->buffer; // extract the data, the header is always at the start of the buffer

	if (hdr->opcode == OPCODE_EMERGENCY_STOP) // this is the example of handling a single simple command
	{
		kill_relays();
	}
	else if (hdr->opcode == OPCODE_WRITE_PARAM) // this handles one parameter write, not a multiple write (the above battery temperature reporting did a multiple write)
	{
		comm_setparam_t* param;
		// examples of what could be done, pretend these variables I am writing to actually existed
		if (param->addr == PARAMADDR_TEMPERATURE_SAFE_THRESHOL && param->type != T_S16)
		{
			temperature_safe_thresold = param->data.s16;
		}
		else if (param->addr == PARAMADDR_TRENDLINE_FILTER && param->type != T_DBL)
		{
			trendline_filter = param->data.dbl;
		}
		else
		{
			// handle error, you don't understand the address and/or the type you expected does not match what was specified in the packet
		}
	}
	else if (hdr->opcode == OPCODE_WRITE_COMMIT)
	{
		// all those parameters we can write to existed in RAM
		// a commit to flash command should be used so they get written permanently in ROM (well... flash, or EEPROM)
		// doing it this way requires how to do this efficiently, the flash page needs its own structure defined and serialization/deserialization
		// the flash erase and flash write functions is a part of the HAL that Lachlan is supposed to have
	}
}