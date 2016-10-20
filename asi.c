#include "asi.h"
#include "modbus.h"
#include "modbus_defs.h"

/*
Attention implementors
fill this table with the data you want to poll from the ASI controller
as they arrive, the array asi_data will be filled in the same order as you specify in this table
make sure asi_data is defined to be larger than this table
*/
const uint16_t asi_wantedAddrs[ASI_DATA_STORE_SIZE + 1] = {
	ASIRegAddr_motor_rpm,
	ASIRegAddr_battery_voltage,
	ASIRegAddr_motor_current,
	ASIRegAddr_motor_input_power,
	//ASIRegAddr_battery_current,
	//ASIRegAddr_battery_power,
	ASIRegAddr_throttle_voltage,
	ASIRegAddr_controller_temperature,
	0xFFFF, // terminate table
};

engine_data_t asi_engines[ASI_ENGINE_COUNT]; // publically accessible
uint8_t asi_engineIdx = 0;
uint8_t asi_lastReqIdx = 0;
uint8_t asi_maxReqIdx = 0;
char asi_pendingReply = 0;

char asi_poll(void)
{
	static systmr_t last_time;
	uint16_t data;

	if (asi_pendingReply == 0)
	{
		int32_t reg = -1;
		while (reg < 0)
		{
			uint16_t x;
			asi_lastReqIdx++;
			if (asi_lastReqIdx >= ASI_DATA_STORE_SIZE) {
				asi_lastReqIdx = -1;
				continue;
			}
			x = asi_wantedAddrs[asi_lastReqIdx];
			if (x != 0xFFFF) // not end of table
			{
				if (asi_lastReqIdx > asi_maxReqIdx) {
					asi_maxReqIdx = asi_lastReqIdx;
				}
				reg = x;
				break;
			}
			else
			{
				asi_lastReqIdx = -1;
			}
		}
		ModRTU_RequestParam(asi_engines[asi_engineIdx].dev_addr, reg, 1);
		asi_pendingReply = 1;
		last_time = systmr_nowMillis();
	}
	else
	{
		modbus_rx_res_t r = Modbus_RxTask(&data);
		if (r == MODBUS_RX_DATA || r == MODBUS_RX_TIMEOUT)
		{
			if (r == MODBUS_RX_DATA)
			{
				asi_data[asi_lastReqIdx] = data;
				asi_dataFlag = 1; // signal upper application
				asi_engines[asi_engineIdx].flags |= ASIFLAG_SINGLE_NEW_DATA;
				if (asi_lastReqIdx < asi_maxReqIdx) { // all data obtained
					asi_engines[asi_engineIdx].flags |= ASIFLAG_ALL_NEW_DATA;
					// do next engine
					asi_engineIdx += 1;
					asi_engineIdx %= ASI_ENGINE_COUNT;
				}
				return 1;
			}
			else
			{
				// indicate error to app
				asi_engines[asi_engineIdx].flags |= ASIFLAG_ERROR;
				// do next engine
				asi_engineIdx += 1;
				asi_engineIdx %= ASI_ENGINE_COUNT;
			}
			asi_pendingReply = 0; // signal ready to send
		}
	}
	return 0;
}

// this function sets the dev_addr of all engines, either all the same (using the default starting addr and inc = 0), or incrementing
void asi_swInit(uint8_t start_addr, uint8_t inc)
{
	int i;
	for (i = 0; i < ASI_ENGINE_COUNT; i++)
	{
		asi_engines[i].flags = 0;
		asi_engines[i].dev_addr = start_addr + (inc * i);
	}
}