#include "modbus.h"
#include "systmr.h"

#include <stdlib.h>
#include <string.h>

#define DEFAULT_MODBUS_ADDRESS 1

// TODO: Lachlan to implement low level drivers
extern void modbus_hal_tx(uint8_t* buff, uint8_t len);
extern uint8_t modbus_hal_avail(void);
extern int16_t modbus_hal_read(void);

void ModRTU_Send(uint8_t addr, uint8_t func, uint8_t* buf, uint8_t n)
{
	uint8_t modbus_buff[MODBUS_BUFF_SIZE];
	uint16_t* crc_ptr = (uint16_t*)&modbus_buff[2 + n];
	uint16_t crc;

	if (n + 4 >= MODBUS_BUFF_MAX) {
		// TODO error message
		return;
	}

	modbus_buff[0] = addr;
	modbus_buff[1] = func;
	memcpy(&modbus_buff[2], buf, n);
	crc = ModRTU_CRC(modbus_buff, 2 + n);
	*crc_ptr = crc;

	modbus_hal_tx(modbus_buff, 2 + n + 2);
}

void ModRTU_RequestParam(uint16_t param, uint16_t len)
{
	read_holding_reg_t packet;
	BIGENDIAN16(param, packet.param_h, packet.param_l);
	BIGENDIAN16(len, packet.regcnt_h, packet.regcnt_l);
	ModRTU_Send(DEFAULT_MODBUS_ADDRESS, ModBusFunc_Read_Holding_Registers, (uint8_t*)&packet, sizeof(read_holding_reg_t));
}

void ModRTF_PresetParam(uint16_t param, uint16_t value)
{
	single_reg_t packet;
	BIGENDIAN16(param, packet.param_h, packet.param_l);
	BIGENDIAN16(value, packet.val_h, packet.val_l);
	ModRTU_Send(DEFAULT_MODBUS_ADDRESS, ModBusFunc_Preset_Single_Register, (uint8_t*)&packet, sizeof(preset_single_reg_t));
}

uint16_t ModRTU_CRC(uint8_t *buf, int len)
{
  uint16_t crc = 0xFFFF;

  for (int pos = 0; pos < len; pos++) {
    crc ^= (uint8_t)buf[pos];          // XOR byte into least sig. byte of crc

    for (int i = 8; i != 0; i--) {    // Loop over each bit
      if ((crc & 0x0001) != 0) {      // If the LSB is set
        crc >>= 1;                    // Shift right and XOR 0xA001
        crc ^= 0xA001;
      }
      else                            // Else LSB is not set
        crc >>= 1;                    // Just shift right
    }
  }
  // Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)
  return crc;
}

modbus_rx_res_t modbus_parse(uint8_t* buff, uint8_t len, uint16_t* result)
{
	//uint16_t crc_calc;
	//uint16_t* crc_got;

	uint16_t data;

	if (len <= 4) {
		return MODBUS_RX_ERROR;
	}

	if (buff[0] != ASI_DEVICE_ADDR) {
		return MODBUS_RX_NONE;
	}
	if (buff[1] != ModBusFunc_Read_Holding_Registers) {
		return MODBUS_RX_ERROR;
	}

	/*
	crc_got = (uint16_t*)&(buff[len - 2]);
	crc_calc = ModRTU_CRC(buff, len - 2);
	if (crc_calc != (*crc_got)) {
		return MODBUS_RX_ERROR;
	}
	//*/

	if (buff[2] != 2) { // not right length
		return MODBUS_RX_ERROR;
	}

	data = buff[3];
	data <<= 8;
	data += buff[4];

	*result = data;
	return MODBUS_RX_DATA;
}

modbus_rx_res_t Modbus_RxTask(uint16_t* result)
{
	static systmr_t last_time;
	systmr_t now;
	static uint8_t buff[MODBUS_BUFF_SIZE];
	static uint8_t buff_idx = 0;
	now = systmr_nowMillis();

	// read what is available into a buffer
	while (modbus_hal_avail() > 0) {
		now = systmr_nowMillis();
		uint8_t x = modbus_hal_read();
		buff[buff_idx] = x;

		// code safety check
		if (buff_idx < MODBUS_BUFF_SIZE) {
			buff_idx++;
		}

		last_time = now;

		// parse if packet seems large enough
		// note: hardcoded number is bad but enough for the rPod
		if (buff_idx >= 7) {
			char r = modbus_parse(buff, buff_idx, result);
			buff_idx = 0;
			return r;
		}
	}
	if ((now - last_time) > MODBUS_TIMEOUT)
	{
		// parse if packet seems large enough
		// note: shouldn't actually logically happen here
		// note: hardcoded number is bad but enough for the rPod
		if (buff_idx >= 7) {
			char r = modbus_parse(buff, buff_idx, result);
			buff_idx = 0;
			return r;
		}
		if (buff_idx > 0) {
			buff_idx = 0;
			return MODBUS_RX_TIMEOUT;
		}
		else {
			return MODBUS_RX_NONE;
			buff_idx = 0;
		}
	}
}