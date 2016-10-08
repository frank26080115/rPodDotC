#ifndef _PID_H_
#define _PID_H_

#include "rpod_types.h"

void pid_reset(pid_data_t* data);
float pid_calc(pid_const_t* consts, pid_data_t* state, float curr, float tgt);

#endif