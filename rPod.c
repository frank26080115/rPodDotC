void engine_ctrl()
{
	attitude_t attitude;
	hoverattitude_calculate(&nvm.pod_coords, &laser_readings, &attitude);
	float min_throttle = nvm.engine_min_throttle;

	// do PID for all 4 engines (HDK) or all 4 "corners" (rPod)
	for (i = 0; i < 4; i++)
	{
		engine[i].throttle += pid_calc(&nvm.pid_hover, &engine[i].pid_state, attitude.corner_height[i], desired_z);
	}

	// constrain
	for (i = 0; i < 4; i++)
	{
		float x = engine[i].throttle;
		x = x > 100 ? 100 : (x < min_throttle ? min_throttle : x);
		engine[i].throttle = x;
	}

	// send to DAC
	for (i = 0; i < 4; i++)
	{
		engine[i].setDac(&engine[i]);
	}

	// this can all be done in one big for-loop, but seperating them out gives better chances to insert debug statements
}

void brake_ctrl()
{
	// we have three sources of data for "distance to end of tunnel":
	// * front laser
	// * contrast sensor
	// * accelerometer integration

	// use the minimum between front laser and contrast sensor, and if both of them fail, then use accelerometer integration

	// pos variables range is 0 to 100 for simplicity, 0 to 100 maps to the minimum and maximum of the stepper potentiometer readings
	float brake_pos = pid_calc(&nvm.pid_eddy_braking, current_speed, desired_speed);
	float gimbal_pos = pid_calc(&nvm.pid_gimbal_braking, current_speed, desired_speed);
	float pitch = attitude->pitch > 0 ? 0 : attitude->pitch;
	gimbal_pos -= pid_calc(&nvm.pid_gimbal_compensation, pitch, 0);

	// gimbal should not speed up the pod at all during braking
	gimbal_pos = gimbal_pos < 50 ? 50 : gimbal_pos;

	brake_pos = brake_pos < 0 ? 0 : brake_pos;
	gimbal_pos = gimbal_pos > 100 ? 100 : gimbal_pos;
	brake_pos = brake_pos > 100 ? 100 : brake_pos;
}

float percent_to_stepper_target(stepper_t* stepper, float percentage)
{
	float x = stepper->consts->adc_max - stepper->consts->adc_min;
	x *= percentage;
	x /= 100.0;
	x += stepper->consts->adc_min;
	x = x > stepper->consts->adc_max ? stepper->consts->adc_max : x;
	x = x < stepper->consts->adc_min ? stepper->consts->adc_min : x;
	return x;
}

int main()
{
	// TODO setup pod_coords
	// TODO setup all constants or load from flash
	systmr_t timestamp = systmr_nowMillis();
	while (1)
	{
		// carefully control the rate at which the PID controller operates
		// the code below should have a rate of 100Hz
		if ((systmr_nowMillis() - timestamp) >= 10)
		{
			timestamp = systmr_nowMillis();

			// TODO: get laser readings and pass them in
			engine_ctrl();
		}
	}
	return 0;
}