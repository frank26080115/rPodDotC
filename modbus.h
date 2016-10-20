#ifndef _MODBUS_H_
#define _MODBUS_H_

#include <stdint.h>

typedef struct {
    uint8_t param_h;
    uint8_t param_l;
    uint8_t val_h;
    uint8_t val_l;
} single_reg_t;

typedef enum
{
	MODBUS_RX_NONE,
	MODBUS_RX_DATA,
	MODBUS_RX_TIMEOUT,
	MODBUS_RX_ERROR,
} modbus_rx_res_t;

#define MODBUS_DEFAULT_ADDRESS 1

void ModRTU_Send(uint8_t addr, uint8_t func, uint8_t* buf, uint8_t n);
void ModRTU_RequestParam(uint8_t dev, uint16_t param, uint16_t len);
void ModRTF_PresetParam(uint8_t dev, uint16_t param, uint16_t value);
modbus_rx_res_t Modbus_RxTask(uint8_t dev, uint16_t* result);

#endif