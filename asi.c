#include "asi.h"
#include "modbus.h"
#include "modbus_defs.h"

/*
Attention implementors
fill this table with the data you want to poll from the ASI controller
as they arrive, the array asi_data will be filled in the same order as you specify in this table
make sure asi_data is defined to be larger than this table
*/
#define ASI_DATA_STORE_SIZE 8
const uint16_t asi_wantedAddrs[ASI_DATA_STORE_SIZE + 1] = {
	ASIRegAddr_motor_rpm,
	ASIRegAddr_battery_voltage,
	ASIRegAddr_motor_current,
	ASIRegAddr_motor_input_power,
	ASIRegAddr_battery_current,
	ASIRegAddr_battery_power,
	ASIRegAddr_throttle_voltage,
	ASIRegAddr_controller_temperature,
	0xFFFF, // terminate table
};
uint16_t asi_data[ASI_DATA_STORE_SIZE];

char asi_dataFlag = 0; asi_allDataFlag = 0;
char asi_pendingReply = 0;

char asi_poll(void)
{
	static systmr_t last_time;
	uint16_t data;

	if (asi_pendingReply == 0)
	{
		int32_t addr = -1;
		while (addr < 0)
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
				addr = x;
				break;
			}
			else
			{
				asi_lastReqIdx = -1;
			}
		}
		ModRTU_RequestParam(addr, 1);
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
				if (asi_lastReqIdx < asi_maxReqIdx) { // all data obtained
					asi_allDataFlag = 1; // signal upper application
				}
				return 1;
			}
			asi_pendingReply = 0;
		}
	}
	return 0;
}