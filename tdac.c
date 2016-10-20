/*
tdac means throttle DAC
*/

#include "tdac.h"
#include "amc7812.h"

extern void tdac_hal_xfer(uint8_t i2c_addr, uint8_t* outbuf, uint8_t outlen, uint8_t* inbuf, uint8_t inlen);

void tdac_write(tdac_t* dac, uint8_t cmd, uint16_t val);
uint16_t tdac_read(tdac_t* dac, uint8_t cmd);
char tdac_writeAndVerify(tdac_t* dac, uint8_t cmd, uint16_t val);
uint16_t tdac_xfer(uint8_t i2cAddr, uint8_t cmd, uint16_t data);

void tdac_init(tdac_t* dac)
{
	int i;
	tdac_hwReset();

	// simple check just to make sure I2C is fine
	uint16_t devid = tdac_read(dac, AMC7812_DEV_ID);
	if (devid != 0x1220 && devid != 0x1221) {
		// TODO: assert error
	}

	// set all outputs to zero
	for (i = 0; i < 8; i++) {
		tdac_set(dac, i, 0);
	}

	// sets DAC gains
	if (tdac_writeAndVerify(dac, AMC7812_DAC_GAIN, 0x0000) == 0) {
		// TODO error
	}

	// turn on DACs on all channels
	if (tdac_writeAndVerify(dac, AMC7812_POWER_DOWN, 0x7FFE) == 0) {
		// TODO error
	}

	// this sets continuous DAC mode
	if (tdac_writeAndVerify(dac, AMC7812_DAC_CONF, 0x0000) == 0) {
		// TODO error
	}
}

void tdac_set(tdac_t* dac, uint8_t idx, uint16_t val)
{
	uint8_t adr = AMC7812_DAC_BASE_ADDR + idx;
	tdac_write(dac, adr, val);
}

uint16_t tdac_xfer(uint8_t i2cAddr, uint8_t cmd, uint16_t data)
{
	uint8_t outbuff[2];
	uint8_t inbuff[2];
	uint16_t x;

	if ((cmd & AMC7812_READ_MASK) != 0)
	{
		tdac_hal_xfer(i2cAddr, outbuff, 1, inbuff, 2);
		x  = inbuff[0];
		x += inbuff[1] << 8;
	}
	else // write
	{
		outbuff[0] = data & 0xFF;
		outbuff[1] = data >> 8;
		tdac_hal_xfer(i2cAddr, outbuff, 2, NULL, 0);
	}
	return x;
}

void tdac_write(tdac_t* dac, uint8_t cmd, uint16_t val)
{
	tdac_xfer(dac->i2c_addr, cmd, val);
}

uint16_t tdac_read(tdac_t* dac, uint8_t cmd)
{
	return tdac_xfer(dac->i2cAddr, cmd | AMC7812_READ_MASK);
}

char tdac_writeAndVerify(tdac_t* dac, uint8_t cmd, uint16_t val)
{
	uint16_t r;
	tdac_xfer(dac->i2c_addr, cmd, val);
	r = tdac_read(dac, cmd);
	return (r == val);
}