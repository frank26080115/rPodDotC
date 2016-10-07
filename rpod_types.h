#ifndef _RPOD_TYPES_H_
#define _RPOD_TYPES_H_

#include <stdint.h>

typedef int32_t systmr_t;

typedef float rpod_float_t;
typedef uint16_t rpod_chksum_t;

typedef struct
{
	rpod_float_t p;
	rpod_float_t i;
	rpod_float_t d;
	rpod_float_t i_decay;
	rpod_float_t out_min;
	rpod_float_t out_max;
}
pid_consts_t;

typedef struct
{
	rpod_float_t integral;
	rpod_float_t prev_error;
}
pid_data_t;

typedef struct
{
	int32_t endpoint_mid;
	int32_t endpoint_max;
	int16_t adc_min;
	int16_t adc_mid;
	int16_t adc_max;
}
stepper_const_t;

typedef
{
	int id;
	stepper_const_t* consts;
	int position;
	void (*moveRelative)(void*, int);
	void (*moveAbsolute)(void*, int);
	char (*isMinHit)(void*);
	char (*isMaxHit)(void*);
	uint16_t (*getAdc)(void*);
}
stepper_t;

typedef struct
{
	pid_consts_t pid_roll;
	pid_consts_t pid_pitch;
	pid_consts_t pid_height;
	pid_consts_t pid_gimbal_braking;
	pid_consts_t pid_eddy_braking;
	pid_consts_t pid_gimbal_stepper;
	pid_consts_t pid_brake_stepper;

	int32_t engine_min_throttle;

	rpod_float_t offset_laser_bottom[4];
	rpod_float_t offset_laser_front;

	stepper_consts_t gimble_stepper[4];
	stepper_consts_t brake_stepper[2];

	rpod_chksum_t chksum;
}
nvm_data_t;

#endif