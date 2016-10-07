#ifndef _PID_H_
#define _PID_H_

#include "rpod_types.h"

void pid_reset(pid_data_t* data);
rpod_float_t pid_calc(pid_const_t* consts, pid_data_t* state, rpod_float_t curr, rpod_float_t tgt);

#endif