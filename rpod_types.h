#ifndef _RPOD_TYPES_H_
#define _RPOD_TYPES_H_

#include <stdint.h>

typedef int32_t systmr_t;

typedef float float;
typedef uint16_t rpod_chksum_t;

typedef struct
{
	float p;
	float i;
	float d;
	float i_decay;
	float out_min;
	float out_max;
}
pid_consts_t;

typedef struct
{
	float integral;
	float prev_error;
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
	int target;
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
	pid_consts_t pid_gimbal_compensation;
	pid_consts_t pid_eddy_braking;
	pid_consts_t pid_gimbal_stepper;
	pid_consts_t pid_brake_stepper;

	int32_t engine_min_throttle;
	int32_t engine_max_throttle;

	float offset_laser_bottom[4];
	float offset_laser_front;

	stepper_consts_t gimble_stepper[4];
	stepper_consts_t brake_stepper[2];

	rpod_chksum_t chksum;
}
nvm_data_t;

typedef struct
{
	float base_throttle;
	float fb_diff;
	float lr_diff;
}
mixer_state_t;

#endif